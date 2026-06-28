#include "AlgorithmType.hpp"
#include "Cousty.hpp"
#include "Felzenszwalb.hpp"
#include "IFT.hpp"
#include "image.hpp"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#endif
SegmentationResult run_algorithm(AlgorithmType type, const Image &image,
                                 const CoustyParams &params);

using namespace std;

// Diretorios onde buscar imagens
static const vector<string> IMAGE_DIRS = {"tests/data", "data"};

// Extensoes de imagem suportadas
static const vector<string> IMAGE_EXTS = {".png", ".jpg", ".jpeg",
                                          ".pgm", ".ppm", ".bmp"};

// Lambdas usados no modo sweep ("todas as opcoes")
static const vector<double> SWEEP_LAMBDAS = {5.0,  10.0,  20.0,  30.0, 50.0,
                                             75.0, 100.0, 150.0, 200.0};

/**
 * @brief Verifica se uma extensao e de imagem suportada.
 */
static bool is_image_extension(const string &ext) {
  for (const auto &e : IMAGE_EXTS) {
    if (ext == e)
      return true;
  }
  return false;
}

static bool is_directory(const string &path) {
  struct stat info;
  if (stat(path.c_str(), &info) != 0)
    return false;
  return (info.st_mode & S_IFDIR) != 0;
}

static vector<string> list_files_in_dir(const string &dir) {
  vector<string> files;
  DIR *dp = opendir(dir.c_str());
  if (!dp)
    return files;

  struct dirent *entry;
  while ((entry = readdir(dp)) != nullptr) {
    // d_type nao e confiavel no MinGW; usamos stat para verificar
    // se e um arquivo regular (nao um diretorio).
    string full_path = dir + "/" + entry->d_name;
    if (!is_directory(full_path)) {
      files.emplace_back(entry->d_name);
    }
  }
  closedir(dp);
  return files;
}

static vector<string> find_available_images() {
  vector<string> images;

  for (const auto &dir : IMAGE_DIRS) {
    if (!is_directory(dir)) {
      continue;
    }

    vector<string> names = list_files_in_dir(dir);
    for (const auto &name : names) {
      string ext;
      size_t pos = name.find_last_of('.');
      if (pos != string::npos) {
        ext = name.substr(pos);
        for (auto &c : ext)
          c = static_cast<char>(tolower(c));
        if (is_image_extension(ext)) {
          images.push_back(dir + "/" + name);
        }
      }
    }
  }

  return images;
}

/**
 * @brief Exibe o banner do programa.
 */
static void print_banner() {
  cout << "\n"
       << "========================================================\n"
       << "    Trabalho de Segmentação com diferentes Métodos\n"
       << "========================================================\n"
       << endl;
}

/**
 * @brief Menu de selecao de imagem.
 * @return Caminho da imagem selecionada, ou string vazia se cancelado.
 */
static string select_image() {
  vector<string> images = find_available_images();

  cout << "--- Selecao de Imagem ---\n" << endl;

  if (!images.empty()) {
    cout << "Imagens encontradas:\n" << endl;
    for (size_t i = 0; i < images.size(); ++i) {
      cout << "  " << (i + 1) << ". " << images[i] << endl;
    }
    cout << "  " << (images.size() + 1) << ". Digitar caminho manualmente"
         << endl;
    cout << "  0. Sair\n" << endl;

    cout << "Escolha uma opcao: ";
    int choice;
    cin >> choice;

    if (choice == 0) {
      return "";
    } else if (choice >= 1 && choice <= static_cast<int>(images.size())) {
      return images[choice - 1];
    } else if (choice == static_cast<int>(images.size()) + 1) {
      cout << "Digite o caminho da imagem: ";
      string path;
      cin >> path;
      return path;
    } else {
      cerr << "Opcao invalida." << endl;
      return "";
    }
  } else {
    cout << "Nenhuma imagem encontrada nos diretorios padrao." << endl;
    cout << "Digite o caminho da imagem: ";
    string path;
    cin >> path;
    return path;
  }
}

/**
 * @brief Menu de configuracao do lambda.
 * @return Valor de lambda escolhido, ou -1.0 para o modo sweep (todas as
 * opcoes).
 */
static double select_lambda() {
  cout << "\n--- Nivel de Corte (Lambda) ---\n" << endl;
  cout << "  1. Lambda = 10.0  (muitos segmentos)" << endl;
  cout << "  2. Lambda = 30.0  (moderado - padrao)" << endl;
  cout << "  3. Lambda = 50.0  (poucos segmentos)" << endl;
  cout << "  4. Lambda = 100.0 (muito poucos segmentos)" << endl;
  cout << "  5. Digitar valor personalizado" << endl;
  cout << "  6. Gerar todas as opcoes de lambda (sweep)" << endl;
  cout << "\nEscolha uma opcao [2]: ";

  string line;
  getline(cin, line);

  if (line.empty() || line == "2")
    return 30.0;
  if (line == "1")
    return 10.0;
  if (line == "3")
    return 50.0;
  if (line == "4")
    return 100.0;
  if (line == "5") {
    cout << "Digite o valor de lambda: ";
    double val;
    cin >> val;
    cin.ignore();
    return val;
  }
  if (line == "6")
    return -1.0; // sentinel para sweep

  // Tentar interpretar como numero direto
  try {
    return stod(line);
  } catch (...) {
    cout << "Valor invalido, usando padrao (30.0)." << endl;
    return 30.0;
  }
}

/**
 * @brief Menu de configuracao da conectividade.
 * @return 4 ou 8.
 */
static int select_connectivity() {
  cout << "\n--- Conectividade ---\n" << endl;
  cout << "  1. 4-vizinhanca (horizontal e vertical)" << endl;
  cout << "  2. 8-vizinhanca (inclui diagonais - padrao)" << endl;
  cout << "\nEscolha uma opcao [2]: ";

  string line;
  getline(cin, line);

  if (line.empty() || line == "2")
    return 8;
  if (line == "1")
    return 4;

  cout << "Opcao invalida, usando padrao (8)." << endl;
  return 8;
}

/**
 * @brief Menu de configuracao do saliency map.
 * @return true se deve gerar saliency map.
 */
static bool select_saliency() {
  cout << "\n--- Saliency Map ---\n" << endl;
  cout << "  1. Sim, gerar saliency map (padrao)" << endl;
  cout << "  2. Nao gerar" << endl;
  cout << "\nEscolha uma opcao [1]: ";

  string line;
  getline(cin, line);

  if (line.empty() || line == "1")
    return true;
  if (line == "2")
    return false;

  cout << "Opcao invalida, usando padrao (sim)." << endl;
  return true;
}

/**
 * @brief Menu de configuracao do prefixo de saida.
 * @return Prefixo para arquivos de saida.
 */
static string select_output_prefix() {
  cout << "\n--- Prefixo de Saida ---\n" << endl;
  cout << "Os arquivos serao salvos na pasta 'results' como:\n"
       << "results/<prefixo>_segmentation.png e "
          "results/<prefixo>_saliency.png.\n"
       << endl;
  cout << "Digite o prefixo [output]: ";

  string line;
  getline(cin, line);

  if (line.empty())
    return "output";
  return line;
}

/**
 * @brief Garante que o diretorio de saida existe.
 */
static void ensure_output_dir(const string &out_dir) {
  if (!is_directory(out_dir)) {
#ifdef _WIN32
    _mkdir(out_dir.c_str());
#else
    mkdir(out_dir.c_str(), 0755);
#endif
  }
}

/**
 * @brief Executa o pipeline para um unico lambda e salva os resultados.
 * @return O SegmentationResult produzido.
 */
// SegmentationResult run_algorithm(
//     AlgorithmType type,
//     const Image& image,
//     const CoustyParams& params
// );

static SegmentationResult run_single(AlgorithmType algo, const Image &image,
                                     CoustyParams params,
                                     const string &seg_path,
                                     const string &sal_path) {

  SegmentationResult result = run_algorithm(algo, image, params);

  try {
    save_image(seg_path, result.segmentation_image);
    cout << "  Segmentacao salva em: " << seg_path << endl;
  } catch (const exception &e) {
    cerr << "  Erro ao salvar segmentacao: " << e.what() << endl;
  }

  if (params.compute_saliency && !sal_path.empty()) {
    if (result.saliency_image.width > 0 && result.saliency_image.height > 0 &&
        result.saliency_image.channels > 0) {
      try {
        save_image(sal_path, result.saliency_image);
        cout << "  Saliency map salvo em: " << sal_path << endl;
      } catch (const exception &e) {
        cerr << "  Erro ao salvar saliency map: " << e.what() << endl;
      }
    } else {
      cout << "  Saliency map nao disponivel para este algoritmo." << endl;
    }
  }

  return result;
}

static AlgorithmType select_algorithm() {
  std::cout << "\n--- Algoritmo de Segmentacao ---\n\n";
  std::cout << "  1. Felzenszwalb-Huttenlocher - Artigo A\n";
  std::cout << "  2. Cousty (hierarquico) - Artigo B\n";
  std::cout << "  3. IFT (gradient) - Artigo C\n";
  std::cout << "\nEscolha uma opcao [1]: ";

  std::string line;
  std::getline(std::cin, line);

  if (line.empty() || line == "1")
    return AlgorithmType::FELZENSZWALB;
  if (line == "2")
    return AlgorithmType::COUSTY;
  if (line == "3")
    return AlgorithmType::IFT_GRADIENT;

  std::cout << "Opcao invalida, usando Felzenszwalb-Huttenlocher\n";
  return AlgorithmType::FELZENSZWALB;
}

SegmentationResult run_algorithm(AlgorithmType type, const Image &image,
                                 const CoustyParams &params) {
  switch (type) {

  case AlgorithmType::COUSTY:
    return cousty_segment(image, params);

  case AlgorithmType::IFT_GRADIENT: {
    IFTParams ift_params;
    ift_params.connectivity = params.connectivity;
    ift_params.auto_seeds = true;

    return ift_segment(image, ift_params);
  }

  case AlgorithmType::FELZENSZWALB: {
    FelzenszwalbParams params_fz;
    params_fz.k = 300.0;
    params_fz.min_size = 50;
    params_fz.connectivity = params.connectivity;
    params_fz.sigma = 0.5;
    params_fz.compute_saliency = params.compute_saliency;

    return felzenszwalb_segment(image, params_fz);
  }
  }

  throw std::runtime_error("Algoritmo invalido");
}

int main() {
  print_banner();

  // --- 1. Selecionar imagem ---
  string input_path = select_image();
  if (input_path.empty()) {
    cout << "Saindo." << endl;
    return 0;
  }

  // Limpar buffer do cin apos leitura com >>
  cin.ignore();

  // --- 2. Configurar parametros ---
  CoustyParams params;
  AlgorithmType algo = select_algorithm();
  double lambda_choice = select_lambda();
  params.connectivity = select_connectivity();
  params.compute_saliency = select_saliency();
  string output_prefix = select_output_prefix();

  bool sweep_mode = (lambda_choice < 0.0);

  // --- Resumo da configuracao ---
  cout << "\n--- Configuracao ---" << endl;
  cout << "  Imagem:         " << input_path << endl;
  if (sweep_mode) {
    cout << "  Modo:           Sweep de lambda (";
    for (size_t i = 0; i < SWEEP_LAMBDAS.size(); ++i) {
      if (i)
        cout << ", ";
      cout << SWEEP_LAMBDAS[i];
    }
    cout << ")" << endl;
  } else {
    cout << "  Lambda:         " << lambda_choice << endl;
  }
  cout << "  Conectividade:  " << params.connectivity << endl;
  cout << "  Saliency map:   " << (params.compute_saliency ? "sim" : "nao")
       << endl;
  cout << "  Prefixo saida:  " << output_prefix << endl;
  cout << "\nPressione ENTER para iniciar ou Ctrl+C para cancelar...";
  cin.get();

  // --- 3. Carregar imagem (uma unica vez) ---
  cout << "\nCarregando imagem: " << input_path << endl;

  Image image;
  try {
    image = load_image(input_path);
  } catch (const exception &e) {
    cerr << "Erro ao carregar imagem: " << e.what() << endl;
    return 1;
  }

  cout << "Imagem carregada: " << image.width << " x " << image.height << " ("
       << image.channels << " canais)" << endl;

  string out_dir = "results";
  ensure_output_dir(out_dir);

  // --- 4. Executar pipeline ---
  if (sweep_mode) {
    // Modo sweep: roda para cada lambda predefinido
    cout << "\n=== Modo Sweep: gerando " << SWEEP_LAMBDAS.size()
         << " segmentacoes ===\n"
         << endl;

    // Cabecalho da tabela de resultados
    cout << left << setw(10) << "Lambda" << setw(12) << "Segmentos" << setw(12)
         << "Tempo(ms)"
         << "Arquivo" << endl;
    cout << string(60, '-') << endl;

    for (double lam : SWEEP_LAMBDAS) {
      params.lambda = lam;

      // Formatar sufixo do lambda (sem ponto para nomes de arquivo)
      string lam_tag;
      if (lam == static_cast<double>(static_cast<int>(lam))) {
        lam_tag = to_string(static_cast<int>(lam));
      } else {
        // Manter uma casa decimal
        ostringstream oss;
        oss << fixed << setprecision(1) << lam;
        lam_tag = oss.str();
        // Substituir '.' por 'p'
        replace(lam_tag.begin(), lam_tag.end(), '.', 'p');
      }

      string seg_path = out_dir + "/" + output_prefix + "_lambda" + lam_tag +
                        "_segmentation.png";
      string sal_path = params.compute_saliency
                            ? (out_dir + "/" + output_prefix + "_lambda" +
                               lam_tag + "_saliency.png")
                            : "";

      cout << "\n[Lambda = " << lam << "]" << endl;
      SegmentationResult res =
          run_single(algo, image, params, seg_path, sal_path);

      cout << left << setw(10) << lam << setw(12) << res.num_segments
           << setw(12) << fixed << setprecision(1) << res.elapsed_ms << seg_path
           << defaultfloat << endl;
    }

    cout << "\n=== Sweep concluido! " << SWEEP_LAMBDAS.size()
         << " segmentacoes salvas na pasta '" << out_dir << "'. ===" << endl;

  } else {
    // Modo normal: um unico lambda
    params.lambda = lambda_choice;

    string seg_path = out_dir + "/" + output_prefix + "_segmentation.png";
    string sal_path = params.compute_saliency
                          ? (out_dir + "/" + output_prefix + "_saliency.png")
                          : "";

    cout << "\nExecutando segmentacao..." << endl;
    run_single(algo, image, params, seg_path, sal_path);

    cout << "\nConcluido com sucesso!" << endl;
    cout << "As imagens geradas foram salvas na pasta '" << out_dir << "'."
         << endl;
  }

  return 0;
}

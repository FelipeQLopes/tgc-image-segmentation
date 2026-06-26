#include "Cousty.hpp"
#include "image.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

// Diretorios onde buscar imagens
static const vector<string> IMAGE_DIRS = {
    "tests/data",
    "data"
};

// Extensoes de imagem suportadas
static const vector<string> IMAGE_EXTS = {
    ".png", ".jpg", ".jpeg", ".pgm", ".ppm", ".bmp"
};

/**
 * @brief Verifica se uma extensao e de imagem suportada.
 */
static bool is_image_extension(const string& ext) {
    for (const auto& e : IMAGE_EXTS) {
        if (ext == e) return true;
    }
    return false;
}

/**
 * @brief Busca imagens nos diretorios conhecidos.
 * @return Vetor de caminhos relativos das imagens encontradas.
 */
static vector<string> find_available_images() {
    vector<string> images;

    for (const auto& dir : IMAGE_DIRS) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            continue;
        }
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                string ext = entry.path().extension().string();
                // Converter para lowercase
                for (auto& c : ext) c = static_cast<char>(tolower(c));
                if (is_image_extension(ext)) {
                    images.push_back(entry.path().string());
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
         << "   Segmentacao Hierarquica de Cousty - Pipeline Completo\n"
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
 * @return Valor de lambda escolhido.
 */
static double select_lambda() {
    cout << "\n--- Nivel de Corte (Lambda) ---\n" << endl;
    cout << "  1. Lambda = 10.0  (muitos segmentos)" << endl;
    cout << "  2. Lambda = 30.0  (moderado - padrao)" << endl;
    cout << "  3. Lambda = 50.0  (poucos segmentos)" << endl;
    cout << "  4. Lambda = 100.0 (muito poucos segmentos)" << endl;
    cout << "  5. Digitar valor personalizado" << endl;
    cout << "\nEscolha uma opcao [2]: ";

    string line;
    getline(cin, line);

    if (line.empty() || line == "2") return 30.0;
    if (line == "1") return 10.0;
    if (line == "3") return 50.0;
    if (line == "4") return 100.0;
    if (line == "5") {
        cout << "Digite o valor de lambda: ";
        double val;
        cin >> val;
        cin.ignore();
        return val;
    }

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

    if (line.empty() || line == "2") return 8;
    if (line == "1") return 4;

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

    if (line.empty() || line == "1") return true;
    if (line == "2") return false;

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
         << "results/<prefixo>_segmentation.png e results/<prefixo>_saliency.png.\n" << endl;
    cout << "Digite o prefixo [output]: ";

    string line;
    getline(cin, line);

    if (line.empty()) return "output";
    return line;
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
    params.lambda           = select_lambda();
    params.connectivity     = select_connectivity();
    params.compute_saliency = select_saliency();
    string output_prefix    = select_output_prefix();

    // --- Resumo da configuracao ---
    cout << "\n--- Configuracao ---" << endl;
    cout << "  Imagem:         " << input_path << endl;
    cout << "  Lambda:         " << params.lambda << endl;
    cout << "  Conectividade:  " << params.connectivity << endl;
    cout << "  Saliency map:   " << (params.compute_saliency ? "sim" : "nao") << endl;
    cout << "  Prefixo saida:  " << output_prefix << endl;
    cout << "\nPressione ENTER para iniciar ou Ctrl+C para cancelar...";
    cin.get();

    // --- 3. Carregar imagem ---
    cout << "\nCarregando imagem: " << input_path << endl;

    Image image;
    try {
        image = load_image(input_path);
    } catch (const exception& e) {
        cerr << "Erro ao carregar imagem: " << e.what() << endl;
        return 1;
    }

    cout << "Imagem carregada: " << image.width << " x " << image.height
         << " (" << image.channels << " canais)" << endl;

    // --- 4. Executar pipeline ---
    cout << "\nExecutando segmentacao..." << endl;
    SegmentationResult result = cousty_segment(image, params);

    // --- 5. Salvar resultados ---
    string out_dir = "results";
    if (!fs::exists(out_dir)) {
        fs::create_directory(out_dir);
    }

    string seg_path = out_dir + "/" + output_prefix + "_segmentation.png";
    try {
        save_image(seg_path, result.segmentation_image);
        cout << "Segmentacao salva em: " << seg_path << endl;
    } catch (const exception& e) {
        cerr << "Erro ao salvar segmentacao: " << e.what() << endl;
        return 1;
    }

    if (params.compute_saliency) {
        string sal_path = out_dir + "/" + output_prefix + "_saliency.png";
        try {
            save_image(sal_path, result.saliency_image);
            cout << "Saliency map salvo em: " << sal_path << endl;
        } catch (const exception& e) {
            cerr << "Erro ao salvar saliency map: " << e.what() << endl;
            return 1;
        }
    }

    cout << "\nConcluido com sucesso!" << endl;
    cout << "As imagens geradas foram salvas na pasta '" << out_dir << "'." << endl;
    return 0;
}
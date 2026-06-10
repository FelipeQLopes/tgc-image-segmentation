#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Estrutura Image — armazena pixels em row-major order.
 */
struct Image {
    int width    = 0;
    int height   = 0;
    int channels = 0; // 1 = cinza, 3 = RGB
    std::vector<uint8_t> pixels;

    // Construtor padrao
    Image() = default;

    /**
     * @brief Construtor com dimensoes (aloca o vetor zerado).
     * @param w Largura da imagem.
     * @param h Altura da imagem.
     * @param c Numero de canais (1 = cinza, 3 = RGB).
     */
    Image(int w, int h, int c)
        : width(w), height(h), channels(c),
          pixels(static_cast<size_t>(w) * h * c, 0) {}

    /**
     * @brief Acesso de leitura a um componente do pixel (x, y).
     * @param x Coluna do pixel.
     * @param y Linha do pixel.
     * @param c Canal (0 por padrao).
     * @return Valor do componente (0–255).
     * @throws std::out_of_range Se os indices estiverem fora dos limites.
     */
    uint8_t at(int x, int y, int c = 0) const {
        if (x < 0 || x >= width || y < 0 || y >= height ||
            c < 0 || c >= channels) {
            throw std::out_of_range("Image::at — indice fora dos limites");
        }
        return pixels[static_cast<size_t>((y * width + x) * channels + c)];
    }

    /**
     * @brief Acesso de escrita a um componente do pixel (x, y).
     * @param x Coluna do pixel.
     * @param y Linha do pixel.
     * @param c Canal.
     * @param value Valor a ser escrito (0–255).
     * @throws std::out_of_range Se os indices estiverem fora dos limites.
     */
    void set(int x, int y, int c, uint8_t value) {
        if (x < 0 || x >= width || y < 0 || y >= height ||
            c < 0 || c >= channels) {
            throw std::out_of_range("Image::set — indice fora dos limites");
        }
        pixels[static_cast<size_t>((y * width + x) * channels + c)] = value;
    }
};

/**
 * @brief Carrega imagem (PNG, JPG, PGM, PPM) via stb_image.
 * @param filepath Caminho do arquivo de imagem.
 * @return Image preenchida com os dados carregados.
 * @throws std::runtime_error Se o arquivo nao existir ou nao puder ser lido.
 */
Image load_image(const std::string& filepath);

/**
 * @brief Salva imagem em PNG via stb_image_write.
 * @param filepath Caminho do arquivo de saida.
 * @param image Imagem a ser salva (1 ou 3 canais).
 * @throws std::runtime_error Se o numero de canais nao for suportado.
 */
void save_image(const std::string& filepath, const Image& image);

/**
 * @brief Converte imagem RGB (3 canais) para tons de cinza (1 canal).
 * @param image Imagem de entrada (1 ou 3 canais).
 * @return Nova Image com 1 canal, usando Y = 0.299*R + 0.587*G + 0.114*B.
 */
Image to_grayscale(const Image& image);

/**
 * @brief Gera imagem RGB colorizada a partir de labels de segmentacao.
 * @param labels Vetor de labels (um por pixel).
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return Image RGB (3 canais) com uma cor aleatoria distinta por label.
 */
Image colorize_segmentation(const std::vector<int>& labels,
                            int width, int height);

#endif // IMAGE_HPP
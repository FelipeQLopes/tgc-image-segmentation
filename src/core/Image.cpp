#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "core/Image.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <random>
#include <stdexcept>

/**
 * @brief Carrega uma imagem do disco (PNG, JPG, PGM, PPM) via stb_image.
 * @param filepath Caminho do arquivo de imagem.
 * @return Image preenchida com os dados carregados.
 * @throws std::runtime_error Se o arquivo nao existir ou nao puder ser lido.
 */
Image load_image(const std::string& filepath) {
    int w, h, c;
    unsigned char* data = stbi_load(filepath.c_str(), &w, &h, &c, 0);
    if (!data) {
        throw std::runtime_error("load_image: nao foi possivel carregar \"" +
                                 filepath + "\" — " +
                                 stbi_failure_reason());
    }

    Image img(w, h, c);
    std::copy(data, data + static_cast<size_t>(w) * h * c, img.pixels.begin());
    stbi_image_free(data);
    return img;
}

/**
 * @brief Salva uma imagem em PNG via stb_image_write.
 * @param filepath Caminho do arquivo de saida.
 * @param image Imagem a ser salva (1 ou 3 canais).
 * @throws std::runtime_error Se o numero de canais nao for 1 ou 3, ou se a escrita falhar.
 */
void save_image(const std::string& filepath, const Image& image) {
    if (image.channels != 1 && image.channels != 3) {
        throw std::runtime_error(
            "save_image: somente 1 ou 3 canais sao suportados");
    }

    int stride = image.width * image.channels;
    int ok = stbi_write_png(filepath.c_str(),
                            image.width, image.height, image.channels,
                            image.pixels.data(), stride);
    if (!ok) {
        throw std::runtime_error("save_image: falha ao salvar \"" +
                                 filepath + "\"");
    }
}

/**
 * @brief Converte imagem RGB (3 canais) para tons de cinza (1 canal).
 * @param image Imagem de entrada (deve ter 1 ou 3 canais).
 * @return Nova Image com 1 canal, usando Y = 0.299*R + 0.587*G + 0.114*B.
 * @throws std::runtime_error Se a imagem nao tiver 1 ou 3 canais.
 */
Image to_grayscale(const Image& image) {
    if (image.channels == 1) {
        return image; // ja eh cinza, retorna copia
    }
    if (image.channels != 3) {
        throw std::runtime_error(
            "to_grayscale: a imagem deve ter 1 ou 3 canais");
    }

    Image gray(image.width, image.height, 1);

    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            uint8_t r = image.at(x, y, 0);
            uint8_t g = image.at(x, y, 1);
            uint8_t b = image.at(x, y, 2);
            uint8_t lum = static_cast<uint8_t>(
                std::round(0.299 * r + 0.587 * g + 0.114 * b));
            gray.set(x, y, 0, lum);
        }
    }
    return gray;
}

/**
 * @brief Gera imagem RGB colorizada a partir de labels de segmentacao.
 * @param labels Vetor de labels (um por pixel, tamanho = width * height).
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return Image RGB (3 canais) com uma cor aleatoria distinta por label.
 * @throws std::runtime_error Se labels.size() != width * height.
 */
Image colorize_segmentation(const std::vector<int>& labels,
                            int width, int height) {
    if (static_cast<int>(labels.size()) != width * height) {
        throw std::runtime_error(
            "colorize_segmentation: tamanho de labels incompativel "
            "com width * height");
    }

    // Mapeia cada label unico para uma cor RGB aleatoria distinta
    std::map<int, std::array<uint8_t, 3>> color_map;
    std::mt19937 rng(42); // seed fixa para reprodutibilidade
    std::uniform_int_distribution<int> dist(30, 230);

    for (int lbl : labels) {
        if (color_map.find(lbl) == color_map.end()) {
            color_map[lbl] = {{
                static_cast<uint8_t>(dist(rng)),
                static_cast<uint8_t>(dist(rng)),
                static_cast<uint8_t>(dist(rng))
            }};
        }
    }

    Image result(width, height, 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            const auto& cor = color_map[labels[idx]];
            result.set(x, y, 0, cor[0]);
            result.set(x, y, 1, cor[1]);
            result.set(x, y, 2, cor[2]);
        }
    }
    return result;
}

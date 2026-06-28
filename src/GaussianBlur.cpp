#include "GaussianBlur.hpp"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numeric>

// ===========================================================================
//  Entrada/Saída PPM (P6 binário, RGB de 8 bits)
// ===========================================================================

namespace {

inline int reflect101(int p, int size)
{
    if (p < 0)
        return -p;

    if (p >= size)
        return 2 * size - p - 2;

    return p;
}

inline uint8_t safe_at(const Image& img, int x, int y, int ch)
{
    x = reflect101(x, img.width);
    y = reflect101(y, img.height);

    return img.at(x, y, ch);
}

}

// Image load_ppm(const std::string& path)
// {
//     std::ifstream f(path, std::ios::binary);
//     if (!f) throw std::runtime_error("Cannot open: " + path);

//     std::string magic;
//     f >> magic;
//     if (magic != "P6") throw std::runtime_error("Only binary P6 PPM supported");

//     // Ignora comentários
//     char c;
//     f.get(c);
//     while (c == '#') { f.ignore(4096, '\n'); f.get(c); }
//     f.putback(c);

//     int w, h, maxval;
//     f >> w >> h >> maxval;
//     f.get(); // consome o único caractere de espaço em branco após maxval

//     if (maxval != 255)
//         throw std::runtime_error("Only 8-bit PPM (maxval=255) supported");

//     Image img(w, h, 3);
//     std::vector<uint8_t> row_buf(w * 3);

//     for (int r = 0; r < h; ++r) {
//         f.read(reinterpret_cast<char*>(row_buf.data()), w * 3);
//         for (int c2 = 0; c2 < w; ++c2) {
//             img.set(c2, r, 0, row_buf[c2 * 3 + 0]);
//             img.set(c2, r, 1, row_buf[c2 * 3 + 1]);
//             img.set(c2, r, 2, row_buf[c2 * 3 + 2]);
//         }
//     }
//     return img;
// }

// void save_ppm(const std::string& path, const Image& img)
// {
//     std::ofstream f(path, std::ios::binary);
//     if (!f) throw std::runtime_error("Cannot write: " + path);

//     f << "P6\n" << img.width << " " << img.height << "\n255\n";

//     std::vector<uint8_t> row_buf(img.width * img.channels);

//     for (int r = 0; r < img.height; ++r) {
//         for (int c = 0; c < img.width; ++c) {
//             for (int ch = 0; ch < img.channels; ++ch) {
//                 uint8_t v = img.at(c, r, ch);
                
//                 row_buf[c * img.channels + ch] =
//                     static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, v + 0.5f)));
//             }
//         }
//         f.write(reinterpret_cast<char*>(row_buf.data()), img.width * img.channels);
//     }
// }

// ===========================================================================
//  GaussianBlur – construção do kernel
// ===========================================================================

//
// kernel_radius = ceil(2 * sigma)
// kernel_size   = 2 * radius + 1     (sempre ímpar)
//
// Escolher radius = ceil(2·sigma) captura mais de 95% da massa da Gaussiana,
// mesmo para valores grandes de sigma, mantendo o kernel compacto sem
// artefatos visíveis de truncamento.
//

int GaussianBlur::kernel_radius(float sigma){
    return static_cast<int>(std::ceil(2.0f * sigma));
}

//
// Constrói o kernel Gaussiano unidimensional:
//
//   G(x) = exp( -x² / (2·σ²) )        x ∈ [-radius, radius]
//
// O kernel é então normalizado para que a soma dos coeficientes seja igual
// a 1, garantindo que uma imagem constante passe pelo filtro sem alterações.
//

std::vector<float> GaussianBlur::build_kernel(float sigma)
{
    if (sigma <= 0.0f)
        throw std::invalid_argument("sigma must be positive");

    const int radius = kernel_radius(sigma);
    const int size   = 2 * radius + 1;

    std::vector<float> k(size);
    const float denom = 2.0f * sigma * sigma;

    for (int i = 0; i < size; ++i) {
        float x = static_cast<float>(i - radius);   // x ∈ [-r, r]
        k[i] = std::exp(-(x * x) / denom);
    }

    // Normaliza para que a soma seja igual a 1
    float sum = std::accumulate(k.begin(), k.end(), 0.0f);
    for (float& v : k) v /= sum;

    return k;
}

// ===========================================================================
//  GaussianBlur – convolução separável
// ===========================================================================

//
// Passagem horizontal: para cada linha r, aplica a convolução ao pixel (r, c)
// usando o kernel ao longo do eixo das colunas.
//
// Os pixels de borda utilizam Image::safe_at, que aplica preenchimento
// reflect-101: sem escurecimento nas bordas e sem artefatos em forma de anel.
//

void GaussianBlur::convolve_horizontal(const Image& src, Image& dst,
                                       const std::vector<float>& kernel, int ch)
{
    const int radius = static_cast<int>(kernel.size()) / 2;
    const int H = src.height;
    const int W = src.width;

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            float acc = 0.0f;
            for (int ki = 0; ki < static_cast<int>(kernel.size()); ++ki) {
                int col = c + (ki - radius);       // pode estar fora dos limites
                acc += kernel[ki] * safe_at(src, col, r, ch);
            }
            auto value = static_cast<uint8_t>(std::clamp(std::lround(acc), 0L, 255L));
            dst.set(c, r, ch, value);
        }
    }
}

//
// Passagem vertical: para cada coluna c, aplica a convolução ao pixel (r, c)
// usando o kernel ao longo do eixo das linhas.
//
// Opera sobre a saída da passagem horizontal (armazenada em src neste caso).
//
void GaussianBlur::convolve_vertical(const Image& src, Image& dst,
                                     const std::vector<float>& kernel, int ch)
{
    const int radius = static_cast<int>(kernel.size()) / 2;
    const int H = src.height;
    const int W = src.width;

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            float acc = 0.0f;
            for (int ki = 0; ki < static_cast<int>(kernel.size()); ++ki) {
                int row = r + (ki - radius);       // pode estar fora dos limites
                acc += kernel[ki] * safe_at(src, c, row, ch);
            }
            auto value = static_cast<uint8_t>(std::clamp(std::lround(acc), 0L, 255L));
            dst.set(c, r, ch, value);
        }
    }
}

//
// Ponto de entrada público.
//
// Algoritmo
// ---------
//   Para cada canal ch em [0, channels):
//     1. Passagem horizontal: src      → tmp    (convolui cada linha)
//     2. Passagem vertical:   tmp      → result (convolui cada coluna)
//
// O uso de duas passagens unidimensionais separadas fornece complexidade
// O(n·k), onde
//   n = largura × altura    (pixels)
//   k = tamanho do kernel = 2·radius + 1
//
// Uma convolução bidimensional única teria custo O(n·k²), portanto a
// abordagem separável é k vezes mais rápida, o que faz diferença quando
// sigma é grande.
//

Image GaussianBlur::apply(const Image& src, float sigma)
{
    if (sigma <= 0.0f)
        throw std::invalid_argument("sigma must be > 0");

    const std::vector<float> kernel = build_kernel(sigma);

    // Buffer intermediário: mesmas dimensões de src
    Image tmp(src.width, src.height, src.channels);
    Image result(src.width, src.height, src.channels);

    for (int ch = 0; ch < src.channels; ++ch) {
        convolve_horizontal(src, tmp,    kernel, ch);   // passagem 1
        convolve_vertical  (tmp, result, kernel, ch);   // passagem 2
    }

    return result;
}
#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
//  Image – um contêiner simples de pixels independente do número de canais
//
//  Organização (row-major, canais em planos separados):
//      pixel(r, c, ch) = data[ ch * height * width + r * width + c ]
//
//  Quantidade de canais suportada:
//      1  – escala de cinza
//      3  – RGB
// ---------------------------------------------------------------------------

struct Image {
    int width   = 0;
    int height  = 0;
    int channels = 1;                // 1 ou 3
    std::vector<float> data;         // float [0, 255]

    Image() = default;

    Image(int w, int h, int ch)
        : width(w), height(h), channels(ch), data(w * h * ch, 0.0f) {}

    // Acesso ao pixel (com verificação de limites em modo debug e sem verificação em release)
    inline float& at(int row, int col, int ch = 0) {
        return data[ch * height * width + row * width + col];
    }
    inline float at(int row, int col, int ch = 0) const {
        return data[ch * height * width + row * width + col];
    }

    // Borda reflect-101 (espelha sem repetir o pixel da borda):
    //   índice 0 1 2 3 4   largura=5
    //   refletido como: 1 0 | 0 1 2 3 4 | 3 2
    //
    
    inline int clamp_reflect(int idx, int size) const {
        // Espelhamento modular eficiente para qualquer índice negativo ou fora dos limites
        if (idx < 0)    return -idx;
        if (idx >= size) return 2 * size - 2 - idx;
        return idx;
    }

    // Leitura segura com preenchimento por reflexão
    inline float safe_at(int row, int col, int ch = 0) const {
        return at(clamp_reflect(row, height),
                  clamp_reflect(col, width),
                  ch);
    }
};

// ---------------------------------------------------------------------------
//  Utilitário: carregar / salvar PPM (P6) – sem necessidade de biblioteca externa
// ---------------------------------------------------------------------------

Image load_ppm(const std::string& path);
void  save_ppm(const std::string& path, const Image& img);

// ---------------------------------------------------------------------------
//  GaussianBlur
//
//  Implementa a convolução Gaussiana 2-D separável.
//
//  Construção do kernel
//  --------------------
//      kernel_size = 2 * ceil(2 * sigma) + 1        (ímpar, >= 3)
//      G(x)        = exp( -x² / (2·sigma²) )
//      kernel      = G(x) normalizado para que a soma seja igual a 1
//
//  Estratégia de convolução
//  ------------------------
//  Como uma Gaussiana 2-D é separável (G₂D = G₁D ⊗ G₁D), realizamos duas
//  convoluções 1-D em sequência:
//
//      1. Passagem horizontal → cada linha é convoluída com o kernel 1-D
//      2. Passagem vertical   → cada coluna é convoluída com o mesmo kernel 1-D
//
//  Complexidade: O(n · k) em vez de O(n · k²) para um kernel 2-D completo,
//  onde n = número total de pixels e k = kernel_size.
//
//  Tratamento das bordas
//  ---------------------
//  É utilizado preenchimento por espelhamento Reflect-101 para evitar halos
//  artificiais claros ou escuros nas bordas da imagem.
//
//  Suporte a múltiplos canais
//  --------------------------
//  Para imagens com múltiplos canais (por exemplo, RGB), o filtro é aplicado
//  independentemente em cada plano de canal.
// ---------------------------------------------------------------------------

class GaussianBlur {
public:
    // Calcula o kernel Gaussiano 1-D normalizado para o sigma informado.
    // Exposto como público para que possa ser inspecionado ou reutilizado.
    static std::vector<float> build_kernel(float sigma);

    // Retorna o raio do kernel (meia largura, sem contar o centro):
    //   kernel_size = 2 * radius + 1
    static int kernel_radius(float sigma);

    // Aplica o desfoque Gaussiano em src e retorna a imagem resultante.
    // sigma > 0.0f; lança std::invalid_argument caso contrário.
    static Image apply(const Image& src, float sigma);

private:
    // Convolução horizontal em um único canal (src → dst, mesmas dimensões)
    static void convolve_horizontal(const Image& src, Image& dst,
                                    const std::vector<float>& kernel, int ch);

    // Convolução vertical em um único canal (src → dst, mesmas dimensões)
    static void convolve_vertical(const Image& src, Image& dst,
                                  const std::vector<float>& kernel, int ch);
};
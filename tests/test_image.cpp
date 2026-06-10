// test_image.cpp — testes basicos para o modulo Image
// Compilar: g++ -Wall -Wextra -Iinclude src/Image.cpp tests/test_image.cpp -o build/test_image -lm

#include "image.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                                             \
    do {                                                                       \
        std::cout << "  [RUN ] " << (name) << std::endl;                       \
    } while (0)

#define PASS(name)                                                             \
    do {                                                                       \
        std::cout << "  [PASS] " << (name) << std::endl;                       \
        ++tests_passed;                                                        \
    } while (0)

#define FAIL(name, msg)                                                        \
    do {                                                                       \
        std::cout << "  [FAIL] " << (name) << " — " << (msg) << std::endl;    \
        ++tests_failed;                                                        \
    } while (0)

// ---------------------------------------------------------------------------
// 1. Testa construtor e acessores at/set
// ---------------------------------------------------------------------------
static void test_struct_and_accessors() {
    const char* name = "struct e acessores at/set";
    TEST(name);

    Image img(4, 3, 3);  // 4x3 RGB
    assert(img.width == 4);
    assert(img.height == 3);
    assert(img.channels == 3);
    assert(img.pixels.size() == 4u * 3 * 3);

    // Escreve e le de volta
    img.set(2, 1, 0, 100);
    img.set(2, 1, 1, 150);
    img.set(2, 1, 2, 200);
    assert(img.at(2, 1, 0) == 100);
    assert(img.at(2, 1, 1) == 150);
    assert(img.at(2, 1, 2) == 200);

    // Acesso fora dos limites deve lancar excecao
    bool caught = false;
    try { img.at(10, 0, 0); } catch (const std::out_of_range&) { caught = true; }
    assert(caught);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Testa criacao e salvamento de imagem cinza sintetica
// ---------------------------------------------------------------------------
static void test_save_and_load_gray() {
    const char* name = "salvar e carregar imagem cinza (PNG)";
    TEST(name);

    // Cria gradiente horizontal 64x64 cinza
    Image gray(64, 64, 1);
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
            gray.set(x, y, 0, static_cast<uint8_t>(x * 4));  // 0..252

    save_image("build/test_gray.png", gray);

    // Recarrega e verifica dimensoes
    Image loaded = load_image("build/test_gray.png");
    assert(loaded.width == 64);
    assert(loaded.height == 64);
    assert(loaded.channels == 1);

    // Verifica que os pixels sao iguais (PNG eh lossless)
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x)
            assert(loaded.at(x, y, 0) == gray.at(x, y, 0));

    PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Testa criacao e salvamento de imagem RGB sintetica
// ---------------------------------------------------------------------------
static void test_save_and_load_rgb() {
    const char* name = "salvar e carregar imagem RGB (PNG)";
    TEST(name);

    // Cria imagem 32x32 RGB com padroes simples
    Image rgb(32, 32, 3);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            rgb.set(x, y, 0, static_cast<uint8_t>(x * 8));
            rgb.set(x, y, 1, static_cast<uint8_t>(y * 8));
            rgb.set(x, y, 2, 128);
        }
    }

    save_image("build/test_rgb.png", rgb);

    Image loaded = load_image("build/test_rgb.png");
    assert(loaded.width == 32);
    assert(loaded.height == 32);
    assert(loaded.channels == 3);

    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            for (int c = 0; c < 3; ++c)
                assert(loaded.at(x, y, c) == rgb.at(x, y, c));

    PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Testa load_image com arquivo inexistente
// ---------------------------------------------------------------------------
static void test_load_nonexistent() {
    const char* name = "load_image com arquivo inexistente";
    TEST(name);

    bool caught = false;
    try {
        load_image("arquivo_que_nao_existe.png");
    } catch (const std::runtime_error&) {
        caught = true;
    }
    assert(caught);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 5. Testa conversao para cinza
// ---------------------------------------------------------------------------
static void test_to_grayscale() {
    const char* name = "to_grayscale (luminancia)";
    TEST(name);

    // Pixel vermelho puro: Y = 0.299*255 = 76.245 -> 76
    Image rgb(1, 1, 3);
    rgb.set(0, 0, 0, 255);
    rgb.set(0, 0, 1, 0);
    rgb.set(0, 0, 2, 0);

    Image gray = to_grayscale(rgb);
    assert(gray.width == 1);
    assert(gray.height == 1);
    assert(gray.channels == 1);

    uint8_t expected = static_cast<uint8_t>(std::round(0.299 * 255));
    assert(gray.at(0, 0, 0) == expected);

    // Pixel branco: Y = 255
    Image white(1, 1, 3);
    white.set(0, 0, 0, 255);
    white.set(0, 0, 1, 255);
    white.set(0, 0, 2, 255);
    Image gw = to_grayscale(white);
    assert(gw.at(0, 0, 0) == 255);

    // Imagem maior: gera e salva para conferencia visual
    Image big(64, 64, 3);
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x) {
            big.set(x, y, 0, static_cast<uint8_t>(x * 4));
            big.set(x, y, 1, static_cast<uint8_t>(y * 4));
            big.set(x, y, 2, 100);
        }
    Image gray_big = to_grayscale(big);
    save_image("build/test_grayscale_result.png", gray_big);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 6. Testa colorizacao de segmentacao
// ---------------------------------------------------------------------------
static void test_colorize_segmentation() {
    const char* name = "colorize_segmentation";
    TEST(name);

    int w = 8, h = 8;
    std::vector<int> labels(w * h);

    // 4 quadrantes com labels distintos (0–3)
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            labels[y * w + x] = (y < h / 2 ? 0 : 2) + (x < w / 2 ? 0 : 1);

    Image seg = colorize_segmentation(labels, w, h);
    assert(seg.width == w);
    assert(seg.height == h);
    assert(seg.channels == 3);

    // Cada quadrante deve ter cor uniforme e diferente dos outros
    auto pixel_color = [&](int x, int y) -> uint32_t {
        return (seg.at(x, y, 0) << 16) | (seg.at(x, y, 1) << 8) | seg.at(x, y, 2);
    };

    uint32_t c0 = pixel_color(0, 0);
    uint32_t c1 = pixel_color(w - 1, 0);
    uint32_t c2 = pixel_color(0, h - 1);
    uint32_t c3 = pixel_color(w - 1, h - 1);

    assert(c0 != c1);
    assert(c0 != c2);
    assert(c0 != c3);
    assert(c1 != c2);
    assert(c1 != c3);
    assert(c2 != c3);

    // Pixels no mesmo quadrante devem ter a mesma cor
    assert(pixel_color(0, 0) == pixel_color(1, 1));
    assert(pixel_color(w - 1, 0) == pixel_color(w - 2, 0));

    // Salva para conferencia visual
    // Versao maior para melhor visualizacao
    int bw = 64, bh = 64;
    std::vector<int> big_labels(bw * bh);
    for (int y = 0; y < bh; ++y)
        for (int x = 0; x < bw; ++x)
            big_labels[y * bw + x] = (y < bh / 2 ? 0 : 2) + (x < bw / 2 ? 0 : 1);
    Image seg_big = colorize_segmentation(big_labels, bw, bh);
    save_image("build/test_colorize_result.png", seg_big);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 7. Testa tamanho incompativel em colorize_segmentation
// ---------------------------------------------------------------------------
static void test_colorize_bad_size() {
    const char* name = "colorize_segmentation com tamanho incompativel";
    TEST(name);

    std::vector<int> labels = {0, 1, 2};
    bool caught = false;
    try {
        colorize_segmentation(labels, 4, 4);
    } catch (const std::runtime_error&) {
        caught = true;
    }
    assert(caught);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 8. Testa carregamento de imagem real RGB (landscape.png)
// ---------------------------------------------------------------------------
static void test_load_real_rgb() {
    const char* name = "carregar imagem real RGB (landscape.png)";
    TEST(name);

    Image img = load_image("tests/data/landscape.png");

    // Deve ter dimensoes positivas e 3 ou 4 canais (RGB ou RGBA)
    assert(img.width > 0);
    assert(img.height > 0);
    assert(img.channels >= 3);
    assert(static_cast<int>(img.pixels.size()) == img.width * img.height * img.channels);

    std::cout << "    -> landscape.png: " << img.width << "x" << img.height
              << " (" << img.channels << " canais)" << std::endl;

    // Converte para cinza e salva
    // Se vier com 4 canais (RGBA), criamos versao RGB antes
    Image rgb = img;
    if (img.channels == 4) {
        rgb = Image(img.width, img.height, 3);
        for (int y = 0; y < img.height; ++y)
            for (int x = 0; x < img.width; ++x)
                for (int c = 0; c < 3; ++c)
                    rgb.set(x, y, c, img.at(x, y, c));
    }

    Image gray = to_grayscale(rgb);
    assert(gray.channels == 1);
    assert(gray.width == img.width);
    assert(gray.height == img.height);

    save_image("build/landscape_gray.png", gray);
    save_image("build/landscape_copy.png", rgb);

    // Recarrega a copia salva e verifica que bate
    Image reloaded = load_image("build/landscape_copy.png");
    assert(reloaded.width == rgb.width);
    assert(reloaded.height == rgb.height);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 9. Testa carregamento de imagem real cinza (cat_gray.png)
// ---------------------------------------------------------------------------
static void test_load_real_gray() {
    const char* name = "carregar imagem real cinza (cat_gray.png)";
    TEST(name);

    Image img = load_image("tests/data/cat_gray.png");

    assert(img.width > 0);
    assert(img.height > 0);
    assert(static_cast<int>(img.pixels.size()) == img.width * img.height * img.channels);

    std::cout << "    -> cat_gray.png: " << img.width << "x" << img.height
              << " (" << img.channels << " canais)" << std::endl;

    // Se veio como RGB (3 canais), converte para cinza
    Image gray = img;
    if (img.channels >= 3) {
        // Se RGBA, extrai RGB primeiro
        Image rgb = img;
        if (img.channels == 4) {
            rgb = Image(img.width, img.height, 3);
            for (int y = 0; y < img.height; ++y)
                for (int x = 0; x < img.width; ++x)
                    for (int c = 0; c < 3; ++c)
                        rgb.set(x, y, c, img.at(x, y, c));
        }
        gray = to_grayscale(rgb);
    }

    assert(gray.channels == 1);
    save_image("build/cat_gray_saved.png", gray);

    // Recarrega e verifica
    Image reloaded = load_image("build/cat_gray_saved.png");
    assert(reloaded.width == gray.width);
    assert(reloaded.height == gray.height);
    assert(reloaded.channels == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\n=== Testes do modulo Image ===" << std::endl;

    test_struct_and_accessors();
    test_save_and_load_gray();
    test_save_and_load_rgb();
    test_load_nonexistent();
    test_to_grayscale();
    test_colorize_segmentation();
    test_colorize_bad_size();
    test_load_real_rgb();
    test_load_real_gray();

    std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
              << tests_failed << " falharam ---\n" << std::endl;

    return tests_failed > 0 ? 1 : 0;
}

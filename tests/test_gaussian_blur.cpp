// test_gaussian_blur.cpp — testes para o modulo GaussianBlur
// Compilar:
//   g++ -std=c++17 -O2 -Wall -Wextra -o test_gaussian test_gaussian_blur.cpp GaussianBlur.cpp

#include "GaussianBlur.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>

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

static const float EPS = 1e-5f;

static bool approx(float a, float b, float tol = EPS) {
    return std::abs(a - b) <= tol;
}

// ---------------------------------------------------------------------------
// Geradores de imagens sinteticas
// ---------------------------------------------------------------------------

// Cinza: metade esquerda preta (0), metade direita branca (255)
static Image make_gray_step(int w = 128, int h = 128)
{
    Image img(w, h, 1);
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c)
            img.at(r, c, 0) = (c < w / 2) ? 0.0f : 255.0f;
    return img;
}

// Cinza: pixel isolado no centro (resposta ao impulso)
static Image make_gray_impulse(int w = 64, int h = 64)
{
    Image img(w, h, 1);
    img.at(h / 2, w / 2, 0) = 255.0f;
    return img;
}

// Cinza: tabuleiro de xadrez (padrao de alta frequencia)
static Image make_checkerboard(int w = 128, int h = 128, int cell = 8)
{
    Image img(w, h, 1);
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c)
            img.at(r, c, 0) = (((r / cell) + (c / cell)) % 2 == 0) ? 255.0f : 0.0f;
    return img;
}

// Colorida: quatro quadrantes (vermelho, verde, azul, branco)
static Image make_color_quads(int w = 128, int h = 128)
{
    Image img(w, h, 3);
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            bool top  = r < h / 2;
            bool left = c < w / 2;
            img.at(r, c, 0) = ( top &&  left) ? 255.0f :
                               (!top && !left) ? 255.0f : 0.0f;   // R
            img.at(r, c, 1) = ( top && !left) ? 255.0f :
                               (!top && !left) ? 255.0f : 0.0f;   // G
            img.at(r, c, 2) = (!top &&  left) ? 255.0f :
                               (!top && !left) ? 255.0f : 0.0f;   // B
        }
    }
    return img;
}

// Constante 3x3: todos os pixels com valor 100
static Image make_constant(float value = 100.0f)
{
    Image img(3, 3, 1);
    for (float& v : img.data) v = value;
    return img;
}

// ---------------------------------------------------------------------------
// 1. Testa tamanho e raio do kernel
// ---------------------------------------------------------------------------
//
//   kernel_size = 2 * ceil(2 * sigma) + 1   (sempre impar)
//   sigma=0.8 -> radius=2 -> size=5
//   sigma=2.0 -> radius=4 -> size=9
//
static void test_kernel_size() {
    const char* name = "tamanho e raio do kernel";
    TEST(name);

    // sigma=0.8: ceil(1.6)=2 -> size=5
    assert(GaussianBlur::kernel_radius(0.8f) == 2);
    assert(static_cast<int>(GaussianBlur::build_kernel(0.8f).size()) == 5);

    // sigma=1.0: ceil(2.0)=2 -> size=5
    assert(GaussianBlur::kernel_radius(1.0f) == 2);
    assert(static_cast<int>(GaussianBlur::build_kernel(1.0f).size()) == 5);

    // sigma=2.0: ceil(4.0)=4 -> size=9
    assert(GaussianBlur::kernel_radius(2.0f) == 4);
    assert(static_cast<int>(GaussianBlur::build_kernel(2.0f).size()) == 9);

    // sigma=5.0: ceil(10.0)=10 -> size=21
    assert(GaussianBlur::kernel_radius(5.0f) == 10);
    assert(static_cast<int>(GaussianBlur::build_kernel(5.0f).size()) == 21);

    std::cout << "    -> sigma=0.8: radius=" << GaussianBlur::kernel_radius(0.8f)
              << "  size=" << GaussianBlur::build_kernel(0.8f).size() << std::endl;
    std::cout << "    -> sigma=2.0: radius=" << GaussianBlur::kernel_radius(2.0f)
              << "  size=" << GaussianBlur::build_kernel(2.0f).size() << std::endl;
    std::cout << "    -> sigma=5.0: radius=" << GaussianBlur::kernel_radius(5.0f)
              << "  size=" << GaussianBlur::build_kernel(5.0f).size() << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Testa normalizacao do kernel (soma == 1)
// ---------------------------------------------------------------------------
static void test_kernel_normalization() {
    const char* name = "normalizacao do kernel (soma == 1)";
    TEST(name);

    for (float sigma : {0.5f, 0.8f, 1.0f, 2.0f, 5.0f}) {
        auto k = GaussianBlur::build_kernel(sigma);
        float sum = std::accumulate(k.begin(), k.end(), 0.0f);
        assert(approx(sum, 1.0f));
        std::cout << "    -> sigma=" << sigma << "  sum=" << std::fixed
                  << std::setprecision(7) << sum << std::endl;
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Testa simetria do kernel
// ---------------------------------------------------------------------------
//
//   G(x) = exp(-x^2 / 2s^2) e simetrica: G(-x) == G(x)
//   Portanto kernel[i] deve ser igual a kernel[size-1-i]
//
static void test_kernel_symmetry() {
    const char* name = "simetria do kernel";
    TEST(name);

    for (float sigma : {0.8f, 1.5f, 3.0f}) {
        auto k = GaussianBlur::build_kernel(sigma);
        for (size_t i = 0; i < k.size() / 2; ++i)
            assert(approx(k[i], k[k.size() - 1 - i]));
        std::cout << "    -> sigma=" << sigma << "  simetrico: sim" << std::endl;
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Testa coeficiente central e decaimento do kernel
// ---------------------------------------------------------------------------
//
//   O coeficiente central (x=0) deve ser o maior.
//   Os coeficientes devem decrescer monotonicamente ao afastar do centro.
//
static void test_kernel_peak_at_center() {
    const char* name = "pico do kernel no centro e decaimento monotono";
    TEST(name);

    auto k = GaussianBlur::build_kernel(2.0f);
    int center = static_cast<int>(k.size()) / 2;

    // Centro e o maior
    for (size_t i = 0; i < k.size(); ++i)
        assert(k[center] >= k[i]);

    // Decaimento monotono a partir do centro (metade direita)
    for (int i = center; i + 1 < static_cast<int>(k.size()); ++i)
        assert(k[i] >= k[i + 1]);

    std::cout << "    -> kernel[centro]=" << std::fixed << std::setprecision(4)
              << k[center] << "  kernel[centro+1]=" << k[center + 1]
              << "  kernel[centro+2]=" << k[center + 2] << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 5. Testa invariancia em imagem constante
// ---------------------------------------------------------------------------
//
//   Filtro linear sobre imagem constante nao altera o valor dos pixels.
//   Valida tanto o centro quanto os cantos (tratamento de borda).
//
static void test_constant_image_invariance() {
    const char* name = "imagem constante permanece inalterada apos blur";
    TEST(name);

    Image img = make_constant(100.0f);
    Image blurred = GaussianBlur::apply(img, 1.0f);

    assert(approx(blurred.at(1, 1, 0), 100.0f, 0.5f));   // centro
    assert(approx(blurred.at(0, 0, 0), 100.0f, 0.5f));   // canto TL
    assert(approx(blurred.at(0, 2, 0), 100.0f, 0.5f));   // canto TR
    assert(approx(blurred.at(2, 0, 0), 100.0f, 0.5f));   // canto BL
    assert(approx(blurred.at(2, 2, 0), 100.0f, 0.5f));   // canto BR

    std::cout << "    -> centro=" << blurred.at(1, 1, 0)
              << "  canto=" << blurred.at(0, 0, 0) << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 6. Testa resposta ao impulso (Point Spread Function)
// ---------------------------------------------------------------------------
//
//   Pixel isolado no centro de imagem preta.
//   Apos blur: resposta deve decrescer monotonicamente a partir do centro
//   e conservar a energia total (soma dos pixels == 255).
//
static void test_impulse_response() {
    const char* name = "resposta ao impulso: decaimento e conservacao de energia";
    TEST(name);

    Image imp = make_gray_impulse(64, 64);
    Image blurred = GaussianBlur::apply(imp, 2.0f);

    float centre = blurred.at(32, 32, 0);
    float off1   = blurred.at(32, 33, 0);   // 1 pixel afastado
    float off3   = blurred.at(32, 35, 0);   // 3 pixels afastados

    // Centro deve ser o pico
    assert(centre >= off1);
    assert(off1   >= off3);

    // Conservacao de energia: soma total deve ser ~255
    float total = 0.0f;
    for (float v : blurred.data) total += v;
    assert(approx(total, 255.0f, 2.0f));

    std::cout << "    -> centro=" << centre << "  1px=" << off1
              << "  3px=" << off3 << "  energia=" << total << std::endl;

    save_ppm("test_impulse_sigma2.ppm", blurred);
    std::cout << "    -> salvo: test_impulse_sigma2.ppm" << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 7. Testa suavizacao de borda degrau (step edge)
// ---------------------------------------------------------------------------
//
//   A fronteira entre as duas regioes fica em x = [63, 64].
//   A media dos dois pixels sobre a fronteira deve ser ~127.5 para qualquer sigma.
//
static void test_step_edge_smoothing() {
    const char* name = "suavizacao da borda degrau";
    TEST(name);

    Image orig = make_gray_step();
    save_ppm("test_gray_original.ppm", orig);

    for (float sigma : {0.8f, 2.0f, 5.0f}) {
        Image blurred = GaussianBlur::apply(orig, sigma);
        std::string fname = "test_gray_sigma" + std::to_string(sigma).substr(0, 3) + ".ppm";
        save_ppm(fname, blurred);

        float left_px  = blurred.at(64, 63, 0);
        float right_px = blurred.at(64, 64, 0);
        float mid      = (left_px + right_px) * 0.5f;
        assert(approx(mid, 127.5f, 20.0f));

        std::cout << "    -> sigma=" << sigma << "  mid=" << mid
                  << "  salvo: " << fname << std::endl;
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// 8. Testa tratamento de borda (sem halo escuro nos cantos)
// ---------------------------------------------------------------------------
//
//   Imagem toda branca (255): apos blur com qualquer sigma,
//   os cantos devem permanecer em 255 (reflect padding nao escurece bordas).
//
static void test_border_no_halo() {
    const char* name = "bordas sem halo escuro (reflect padding)";
    TEST(name);

    Image white(64, 64, 1);
    for (float& v : white.data) v = 255.0f;

    Image blurred = GaussianBlur::apply(white, 3.0f);

    float tl = blurred.at(0,  0,  0);
    float tr = blurred.at(0,  63, 0);
    float bl = blurred.at(63, 0,  0);
    float br = blurred.at(63, 63, 0);

    assert(approx(tl, 255.0f, 1.0f));
    assert(approx(tr, 255.0f, 1.0f));
    assert(approx(bl, 255.0f, 1.0f));
    assert(approx(br, 255.0f, 1.0f));

    std::cout << "    -> TL=" << tl << "  TR=" << tr
              << "  BL=" << bl << "  BR=" << br << std::endl;

    save_ppm("test_edge_sigma3.ppm", blurred);
    std::cout << "    -> salvo: test_edge_sigma3.ppm" << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 9. Testa independencia dos canais em imagem colorida
// ---------------------------------------------------------------------------
//
//   No quadrante superior-esquerdo da imagem de quadrantes coloridos,
//   apenas o canal R deve ser ativo (255). Os canais G e B devem ser ~0
//   em pixels distantes das fronteiras.
//
static void test_color_channel_independence() {
    const char* name = "independencia dos canais RGB";
    TEST(name);

    Image orig = make_color_quads();
    save_ppm("test_color_original.ppm", orig);

    for (float sigma : {1.5f, 4.0f}) {
        Image blurred = GaussianBlur::apply(orig, sigma);
        std::string fname = "test_color_sigma" + std::to_string(sigma).substr(0, 3) + ".ppm";
        save_ppm(fname, blurred);
        std::cout << "    -> salvo: " << fname << std::endl;
    }

    // Pixel (32,32) esta no centro do quadrante vermelho (longe das bordas)
    Image blurred = GaussianBlur::apply(orig, 1.0f);
    float g_val = blurred.at(32, 32, 1);   // canal G no quadrante vermelho
    float b_val = blurred.at(32, 32, 2);   // canal B no quadrante vermelho
    assert(approx(g_val, 0.0f, 2.0f));
    assert(approx(b_val, 0.0f, 2.0f));

    std::cout << "    -> G no quadrante vermelho=" << g_val
              << "  B no quadrante vermelho=" << b_val << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 10. Testa que sigma controla o grau de suavizacao
// ---------------------------------------------------------------------------
//
//   Medido pelo desvio padrao dos pixels: quanto maior o sigma,
//   menor o contraste residual na imagem borrada.
//   Verificacao: stddev(original) >= stddev(s=0.8) >= stddev(s=3) >= stddev(s=8)
//
static void test_sigma_controls_smoothing() {
    const char* name = "sigma controla grau de suavizacao (desvio padrao decresce)";
    TEST(name);

    Image board = make_checkerboard();
    save_ppm("test_checker_original.ppm", board);

    Image b1 = GaussianBlur::apply(board, 0.8f);
    Image b2 = GaussianBlur::apply(board, 3.0f);
    Image b3 = GaussianBlur::apply(board, 8.0f);
    save_ppm("test_checker_sigma0.8.ppm", b1);
    save_ppm("test_checker_sigma3.0.ppm", b2);
    save_ppm("test_checker_sigma8.0.ppm", b3);

    auto stddev = [](const Image& img) {
        float mean = 0.0f;
        for (float v : img.data) mean += v;
        mean /= static_cast<float>(img.data.size());
        float var = 0.0f;
        for (float v : img.data) var += (v - mean) * (v - mean);
        return std::sqrt(var / static_cast<float>(img.data.size()));
    };

    float sd0 = stddev(board);
    float sd1 = stddev(b1);
    float sd2 = stddev(b2);
    float sd3 = stddev(b3);

    std::cout << "    -> stddev original=" << std::fixed << std::setprecision(2) << sd0
              << "  sigma=0.8: " << sd1
              << "  sigma=3.0: " << sd2
              << "  sigma=8.0: " << sd3 << std::endl;

    assert(sd0 >= sd1);
    assert(sd1 >= sd2);
    assert(sd2 >= sd3);

    PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\n=== Testes do modulo GaussianBlur ===" << std::endl;

    test_kernel_size();
    test_kernel_normalization();
    test_kernel_symmetry();
    test_kernel_peak_at_center();
    test_constant_image_invariance();
    test_impulse_response();
    test_step_edge_smoothing();
    test_border_no_halo();
    test_color_channel_independence();
    test_sigma_controls_smoothing();

    std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
              << tests_failed << " falharam ---\n" << std::endl;

    return tests_failed > 0 ? 1 : 0;
}

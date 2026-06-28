// test_cousty.cpp — testes para o pipeline completo de Cousty
// Compilar:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/Image.cpp src/Graph.cpp
//   src/DisjointSet.cpp src/Kruskal.cpp src/Hierarchy.cpp src/SaliencyMap.cpp
//   src/Cousty.cpp tests/test_cousty.cpp -o build/test_cousty -lm

#include "algorithms/Cousty.hpp"
#include "core/Image.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers para Relatorio de Testes
// ---------------------------------------------------------------------------
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                                             \
  do {                                                                         \
    std::cout << "  [RUN ] " << (name) << std::endl;                           \
  } while (0)

#define PASS(name)                                                             \
  do {                                                                         \
    std::cout << "  [PASS] " << (name) << std::endl;                           \
    ++tests_passed;                                                            \
  } while (0)

// ---------------------------------------------------------------------------
// Helper: cria imagem sintetica 4x4 cinza com gradiente
// ---------------------------------------------------------------------------
// Pixel (x,y) = x * 20 + y * 30
// Resulta em:
//   (0,0)=0   (1,0)=20  (2,0)=40  (3,0)=60
//   (0,1)=30  (1,1)=50  (2,1)=70  (3,1)=90
//   (0,2)=60  (1,2)=80  (2,2)=100 (3,2)=120
//   (0,3)=90  (1,3)=110 (2,3)=130 (3,3)=150
static Image create_gradient_4x4() {
    Image img(4, 4, 1);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            uint8_t val = static_cast<uint8_t>(x * 20 + y * 30);
            img.set(x, y, 0, val);
        }
    }
    return img;
}

// ---------------------------------------------------------------------------
// Helper: cria imagem sintetica 3x3 RGB com duas regioes
// ---------------------------------------------------------------------------
// Regiao esquerda (x<2): vermelho (200, 50, 50)
// Regiao direita (x>=2): azul (50, 50, 200)
static Image create_bicolor_3x3() {
    Image img(3, 3, 3);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (x < 2) {
                img.set(x, y, 0, 200); // R
                img.set(x, y, 1, 50);  // G
                img.set(x, y, 2, 50);  // B
            } else {
                img.set(x, y, 0, 50);  // R
                img.set(x, y, 1, 50);  // G
                img.set(x, y, 2, 200); // B
            }
        }
    }
    return img;
}

// ---------------------------------------------------------------------------
// 1. Pipeline ponta a ponta com imagem sintetica cinza
// ---------------------------------------------------------------------------
static void test_pipeline_grayscale() {
    const char* name = "Pipeline ponta a ponta — imagem 4x4 cinza";
    TEST(name);

    Image img = create_gradient_4x4();

    CoustyParams params;
    params.lambda = 30.0;
    params.connectivity = 4;
    params.compute_saliency = true;

    SegmentationResult result = cousty_segment(img, params);

    // Deve ter segmentos validos

    // Deve ter pelo menos 1 segmento
    assert(result.num_segments >= 1);

    // Tempo deve ser positivo
    assert(result.elapsed_ms > 0.0);

    // Imagem de segmentacao deve ter dimensoes corretas
    assert(result.segmentation_image.width == 4);
    assert(result.segmentation_image.height == 4);
    assert(result.segmentation_image.channels == 3);

    // Saliency map deve existir
    assert(result.saliency_image.width == 4);
    assert(result.saliency_image.height == 4);
    assert(result.saliency_image.channels == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Pipeline com imagem RGB bicolor
// ---------------------------------------------------------------------------
static void test_pipeline_rgb() {
    const char* name = "Pipeline ponta a ponta — imagem 3x3 RGB bicolor";
    TEST(name);

    Image img = create_bicolor_3x3();

    CoustyParams params;
    params.lambda = 50.0;
    params.connectivity = 8;
    params.compute_saliency = true;

    SegmentationResult result = cousty_segment(img, params);


    assert(result.num_segments >= 1);
    assert(result.elapsed_ms >= 0.0);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Lambda baixo -> muitos segmentos, lambda alto -> poucos
// ---------------------------------------------------------------------------
static void test_lambda_monotonicity() {
    const char* name = "Lambda baixo -> muitos segmentos, alto -> poucos";
    TEST(name);

    Image img = create_gradient_4x4();

    CoustyParams params;
    params.connectivity = 4;
    params.compute_saliency = false;

    // Lambda muito baixo
    params.lambda = 0.0;
    SegmentationResult low = cousty_segment(img, params);

    // Lambda medio
    params.lambda = 30.0;
    SegmentationResult mid = cousty_segment(img, params);

    // Lambda muito alto
    params.lambda = 1e6;
    SegmentationResult high = cousty_segment(img, params);

    // low.num_segments >= mid.num_segments >= high.num_segments
    assert(low.num_segments >= mid.num_segments);
    assert(mid.num_segments >= high.num_segments);

    // Lambda 0 -> cada pixel e um segmento (pesos todos > 0 na imagem gradiente)
    assert(low.num_segments == 16);

    // Lambda muito alto -> 1 segmento
    assert(high.num_segments == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Saliency map opcional — sem saliency
// ---------------------------------------------------------------------------
static void test_no_saliency() {
    const char* name = "Saliency map opcional — desativado";
    TEST(name);

    Image img = create_gradient_4x4();

    CoustyParams params;
    params.lambda = 20.0;
    params.connectivity = 4;
    params.compute_saliency = false;

    SegmentationResult result = cousty_segment(img, params);

    // Saliency image deve estar vazia (dimensoes 0)
    assert(result.saliency_image.width == 0);
    assert(result.saliency_image.height == 0);

    // Segmentacao deve funcionar normalmente
    assert(result.num_segments >= 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 5. Conectividade 4 vs 8 — podem diferir
// ---------------------------------------------------------------------------
static void test_connectivity_4_vs_8() {
    const char* name = "Conectividade 4 vs 8 — resultados podem diferir";
    TEST(name);

    Image img = create_gradient_4x4();

    CoustyParams params4;
    params4.lambda = 25.0;
    params4.connectivity = 4;
    params4.compute_saliency = false;

    CoustyParams params8;
    params8.lambda = 25.0;
    params8.connectivity = 8;
    params8.compute_saliency = false;

    SegmentationResult result4 = cousty_segment(img, params4);
    SegmentationResult result8 = cousty_segment(img, params8);

    // Ambos devem produzir resultados validos
    assert(result4.num_segments >= 1);
    assert(result8.num_segments >= 1);

    // 8-vizinhanca tende a produzir menos segmentos (mais conexoes)
    // mas nao e garantido para todos os lambdas
    // Apenas verificamos que ambos funcionam

    PASS(name);
}

// ---------------------------------------------------------------------------
// 6. Imagem uniforme — todos iguais
// ---------------------------------------------------------------------------
static void test_relative_lambda_scale() {
    const char* name = "Lambda alto deve ser interpretado em escala relativa da MST";
    TEST(name);

    Image img(2, 2, 1);
    img.set(0, 0, 0, 0);
    img.set(1, 0, 0, 40);
    img.set(0, 1, 0, 80);
    img.set(1, 1, 0, 120);

    CoustyParams params;
    params.lambda = 30.0;
    params.connectivity = 4;
    params.compute_saliency = false;

    SegmentationResult result = cousty_segment(img, params);

    // Os pesos de aresta são pequenos comparados a 30 em valor absoluto;
    // o corte deve usar uma escala relativa da MST, preservando mais regiões.
    assert(result.num_segments >= 2);

    PASS(name);
}

static void test_uniform_image() {
    const char* name = "Imagem uniforme — todas as arestas com peso 0";
    TEST(name);

    Image img(3, 3, 1);
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            img.set(x, y, 0, 128);
        }
    }

    CoustyParams params;
    params.lambda = 0.0;
    params.connectivity = 4;
    params.compute_saliency = false;

    SegmentationResult result = cousty_segment(img, params);

    // Todos os pesos sao 0, entao merge_level=0 para todos
    // cut_at_level(0) com merge_level <= lambda -> tudo junto
    assert(result.num_segments == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\n=== Testes do pipeline Cousty ===" << std::endl;

    test_pipeline_grayscale();
    test_pipeline_rgb();
    test_lambda_monotonicity();
    test_no_saliency();
    test_connectivity_4_vs_8();
    test_relative_lambda_scale();
    test_uniform_image();

    std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
              << tests_failed << " falharam ---\n"
              << std::endl;

    return tests_failed > 0 ? 1 : 0;
}

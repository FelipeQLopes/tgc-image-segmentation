// test_graph.cpp — testes para o modulo Graph (ImageGraph)
// Compilar:
//   g++ -Wall -Wextra -Iinclude src/Image.cpp src/Graph.cpp tests/test_graph.cpp -o build/test_graph -lm

#include "Graph.hpp"
#include "image.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
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

static const double EPS = 1e-9;

static bool approx(double a, double b) {
    return std::abs(a - b) < EPS;
}

// ---------------------------------------------------------------------------
// 1. Testa pixel_to_id e id_to_pixel
// ---------------------------------------------------------------------------
static void test_coordinate_conversion() {
    const char* name = "pixel_to_id e id_to_pixel";
    TEST(name);

    Image img(5, 4, 1);   // 5 colunas, 4 linhas
    ImageGraph g(img, Connectivity::FOUR);

    // pixel_to_id: id = y * width + x
    assert(g.pixel_to_id(0, 0) == 0);
    assert(g.pixel_to_id(4, 0) == 4);
    assert(g.pixel_to_id(0, 1) == 5);
    assert(g.pixel_to_id(3, 2) == 13);
    assert(g.pixel_to_id(4, 3) == 19);

    // id_to_pixel: inverso
    int x, y;
    g.id_to_pixel(0, x, y);   assert(x == 0 && y == 0);
    g.id_to_pixel(4, x, y);   assert(x == 4 && y == 0);
    g.id_to_pixel(5, x, y);   assert(x == 0 && y == 1);
    g.id_to_pixel(13, x, y);  assert(x == 3 && y == 2);
    g.id_to_pixel(19, x, y);  assert(x == 4 && y == 3);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Testa numero de vertices
// ---------------------------------------------------------------------------
static void test_num_vertices() {
    const char* name = "numero de vertices (W x H)";
    TEST(name);

    Image img3x3(3, 3, 1);
    ImageGraph g4(img3x3, Connectivity::FOUR);
    assert(g4.num_vertices == 9);

    Image img5x4(5, 4, 1);
    ImageGraph g8(img5x4, Connectivity::EIGHT);
    assert(g8.num_vertices == 20);

    Image img1x1(1, 1, 1);
    ImageGraph g1(img1x1, Connectivity::FOUR);
    assert(g1.num_vertices == 1);
    assert(g1.edges.empty());

    PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Testa contagem de arestas para 3x3 com vizinhanca 4
// ---------------------------------------------------------------------------
//
//  Imagem 3x3 com vizinhanca 4:
//
//    0 - 1 - 2
//    |   |   |
//    3 - 4 - 5
//    |   |   |
//    6 - 7 - 8
//
//  Arestas horizontais: 2 por linha * 3 linhas = 6
//  Arestas verticais:   3 por coluna * 2 = 6
//  Total: 12
//
static void test_edge_count_4conn() {
    const char* name = "contagem de arestas 3x3 vizinhanca-4";
    TEST(name);

    Image img(3, 3, 1);
    ImageGraph g(img, Connectivity::FOUR);

    assert(g.num_vertices == 9);

    // arestas = W*(H-1) + H*(W-1) = 3*2 + 3*2 = 12
    int expected = 3 * (3 - 1) + 3 * (3 - 1);
    assert(static_cast<int>(g.edges.size()) == expected);

    std::cout << "    -> arestas: " << g.edges.size()
              << " (esperado: " << expected << ")" << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Testa contagem de arestas para 3x3 com vizinhanca 8
// ---------------------------------------------------------------------------
//
//  Arestas adicionais (diagonais):
//    diag-inf-dir: (W-1)*(H-1) = 2*2 = 4
//    diag-inf-esq: (W-1)*(H-1) = 2*2 = 4
//  Total: 12 + 4 + 4 = 20
//
static void test_edge_count_8conn() {
    const char* name = "contagem de arestas 3x3 vizinhanca-8";
    TEST(name);

    Image img(3, 3, 1);
    ImageGraph g(img, Connectivity::EIGHT);

    // horizontal + vertical + diag-dir + diag-esq
    // 6 + 6 + 4 + 4 = 20
    int horiz = 3 * (3 - 1);        // 6
    int vert  = 3 * (3 - 1);        // 6
    int diag  = 2 * (3 - 1) * (3 - 1);  // 8 (dir + esq)
    int expected = horiz + vert + diag;  // 20

    assert(static_cast<int>(g.edges.size()) == expected);

    std::cout << "    -> arestas: " << g.edges.size()
              << " (esperado: " << expected << ")" << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 5. Testa ausencia de arestas duplicadas
// ---------------------------------------------------------------------------
static void test_no_duplicate_edges() {
    const char* name = "sem arestas duplicadas";
    TEST(name);

    Image img(4, 4, 1);
    ImageGraph g(img, Connectivity::EIGHT);

    // Verifica que nao ha par (u,v) repetido
    std::set<std::pair<int, int>> seen;
    for (const auto& e : g.edges) {
        auto pair = std::make_pair(std::min(e.u, e.v), std::max(e.u, e.v));
        assert(seen.find(pair) == seen.end());
        seen.insert(pair);
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// 6. Testa pesos em imagem cinza (manual)
// ---------------------------------------------------------------------------
static void test_weights_grayscale() {
    const char* name = "pesos corretos (cinza)";
    TEST(name);

    // Imagem 3x1 cinza: [10, 50, 200]
    Image img(3, 1, 1);
    img.set(0, 0, 0, 10);
    img.set(1, 0, 0, 50);
    img.set(2, 0, 0, 200);

    ImageGraph g(img, Connectivity::FOUR);

    // Deve ter 2 arestas: (0,1) peso=40, (1,2) peso=150
    assert(g.edges.size() == 2);

    // Encontra as arestas
    for (const auto& e : g.edges) {
        if ((e.u == 0 && e.v == 1) || (e.u == 1 && e.v == 0)) {
            assert(approx(e.weight, 40.0));
        } else if ((e.u == 1 && e.v == 2) || (e.u == 2 && e.v == 1)) {
            assert(approx(e.weight, 150.0));
        } else {
            assert(false && "aresta inesperada");
        }
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// 7. Testa pesos em imagem colorida (manual)
// ---------------------------------------------------------------------------
static void test_weights_color() {
    const char* name = "pesos corretos (colorida RGB)";
    TEST(name);

    // Imagem 2x1 RGB
    // Pixel 0: (100, 150, 200)
    // Pixel 1: (110, 140, 180)
    Image img(2, 1, 3);
    img.set(0, 0, 0, 100); img.set(0, 0, 1, 150); img.set(0, 0, 2, 200);
    img.set(1, 0, 0, 110); img.set(1, 0, 1, 140); img.set(1, 0, 2, 180);

    ImageGraph g(img, Connectivity::FOUR);

    assert(g.edges.size() == 1);

    // peso = sqrt((10)^2 + (-10)^2 + (-20)^2) = sqrt(100+100+400) = sqrt(600)
    double expected = std::sqrt(600.0);
    assert(approx(g.edges[0].weight, expected));

    std::cout << "    -> peso: " << g.edges[0].weight
              << " (esperado: " << expected << ")" << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// 8. Testa sort_edges
// ---------------------------------------------------------------------------
static void test_sort_edges() {
    const char* name = "sort_edges (ordem crescente)";
    TEST(name);

    // Imagem 3x1 cinza: [10, 50, 200] => pesos 40, 150
    Image img(3, 1, 1);
    img.set(0, 0, 0, 10);
    img.set(1, 0, 0, 50);
    img.set(2, 0, 0, 200);

    ImageGraph g(img, Connectivity::FOUR);
    g.sort_edges();

    assert(g.edges.size() == 2);
    assert(g.edges[0].weight <= g.edges[1].weight);
    assert(approx(g.edges[0].weight, 40.0));
    assert(approx(g.edges[1].weight, 150.0));

    PASS(name);
}

// ---------------------------------------------------------------------------
// 9. Testa lista de adjacencia
// ---------------------------------------------------------------------------
static void test_adjacency_list() {
    const char* name = "lista de adjacencia bidirecional";
    TEST(name);

    // Imagem 3x1 cinza: vertices 0, 1, 2
    Image img(3, 1, 1);
    ImageGraph g(img, Connectivity::FOUR);

    // Vertice 0: vizinho 1
    assert(g.adj[0].size() == 1);
    assert(g.adj[0][0].first == 1);

    // Vertice 1: vizinhos 0 e 2
    assert(g.adj[1].size() == 2);

    // Vertice 2: vizinho 1
    assert(g.adj[2].size() == 1);
    assert(g.adj[2][0].first == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 10. Testa contagem de arestas com imagem maior (5x4)
// ---------------------------------------------------------------------------
static void test_edge_count_larger() {
    const char* name = "contagem de arestas 5x4";
    TEST(name);

    int W = 5, H = 4;
    Image img(W, H, 1);

    // Vizinhanca 4
    ImageGraph g4(img, Connectivity::FOUR);
    int exp4 = W * (H - 1) + H * (W - 1);  // 5*3 + 4*4 = 15 + 16 = 31
    assert(static_cast<int>(g4.edges.size()) == exp4);

    // Vizinhanca 8
    ImageGraph g8(img, Connectivity::EIGHT);
    int exp8 = exp4 + 2 * (W - 1) * (H - 1);  // 31 + 2*4*3 = 31 + 24 = 55
    assert(static_cast<int>(g8.edges.size()) == exp8);

    std::cout << "    -> 4-conn: " << g4.edges.size() << " (esperado: " << exp4 << ")"
              << "  8-conn: " << g8.edges.size() << " (esperado: " << exp8 << ")"
              << std::endl;

    PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\n=== Testes do modulo Graph ===" << std::endl;

    test_coordinate_conversion();
    test_num_vertices();
    test_edge_count_4conn();
    test_edge_count_8conn();
    test_no_duplicate_edges();
    test_weights_grayscale();
    test_weights_color();
    test_sort_edges();
    test_adjacency_list();
    test_edge_count_larger();

    std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
              << tests_failed << " falharam ---\n" << std::endl;

    return tests_failed > 0 ? 1 : 0;
}

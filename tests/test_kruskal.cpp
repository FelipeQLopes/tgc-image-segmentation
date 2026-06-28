// test_kruskal.cpp — testes para o módulo Kruskal (MST)
// Compilar:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/DisjointSet.cpp src/Kruskal.cpp
//   tests/test_kruskal.cpp -o build/test_kruskal

#include "graph/Kruskal.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers para Relatório de Testes
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

static const double EPS = 1e-9;

static bool approx(double a, double b) { return std::abs(a - b) < EPS; }

// ---------------------------------------------------------------------------
// 1. Testa Grafo Linear Simples (Sem ciclos)
// ---------------------------------------------------------------------------
static void test_linear_graph() {
  const char *name = "Grafo linear sem ciclos";
  TEST(name);

  std::vector<Edge> edges;
  edges.push_back(Edge(0, 1, 1.5));
  edges.push_back(Edge(1, 2, 2.5));

  MSTResult result = kruskal_mst(3, edges);

  assert(result.mst_edges.size() == 2);
  assert(approx(result.total_weight, 4.0));

  // Ambas as arestas originais devem estar na MST
  bool found_01 = false, found_12 = false;
  for (size_t idx = 0; idx < result.mst_edges.size(); ++idx) {
    const Edge &e = result.mst_edges[idx];
    if ((e.u == 0 && e.v == 1) || (e.u == 1 && e.v == 0))
      found_01 = approx(e.weight, 1.5);
    if ((e.u == 1 && e.v == 2) || (e.u == 2 && e.v == 1))
      found_12 = approx(e.weight, 2.5);
  }
  assert(found_01 && found_12);

  PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Testa Grafo Cíclico Simples (Triângulo)
// ---------------------------------------------------------------------------
static void test_triangle_cycle() {
  const char *name = "Grafo cíclico triangular";
  TEST(name);

  std::vector<Edge> edges;
  edges.push_back(Edge(0, 2, 3.0));
  edges.push_back(Edge(0, 1, 1.0));
  edges.push_back(Edge(1, 2, 2.0));

  MSTResult result = kruskal_mst(3, edges);

  assert(result.mst_edges.size() == 2);
  assert(approx(result.total_weight, 3.0));

  // Arestas da MST devem ser (0,1) e (1,2)
  bool found_01 = false, found_12 = false, found_02 = false;
  for (size_t idx = 0; idx < result.mst_edges.size(); ++idx) {
    const Edge &e = result.mst_edges[idx];
    if ((e.u == 0 && e.v == 1) || (e.u == 1 && e.v == 0))
      found_01 = approx(e.weight, 1.0);
    if ((e.u == 1 && e.v == 2) || (e.u == 2 && e.v == 1))
      found_12 = approx(e.weight, 2.0);
    if ((e.u == 0 && e.v == 2) || (e.u == 2 && e.v == 0))
      found_02 = true;
  }
  assert(found_01 && found_12);
  assert(!found_02); // Rejeitada

  PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Testa Grade 3x3 (Representando Imagem Pequena)
// ---------------------------------------------------------------------------
// Grafo:
//  0 - 1 - 2
//  |   |   |
//  3 - 4 - 5
//  |   |   |
//  6 - 7 - 8
//
// Arestas e pesos configurados manualmente:
// (0,1) p=1.0, (1,2) p=10.0, (3,4) p=2.0, (4,5) p=1.0, (6,7) p=1.5, (7,8) p=1.0
// Verticais:
// (0,3) p=2.0, (1,4) p=1.0, (2,5) p=2.0, (3,6) p=10.0, (4,7) p=3.0, (5,8) p=1.5
//
// Peso esperado da MST:
// Arestas na MST (9 vértices -> 8 arestas):
// 1. (0,1) peso 1.0
// 2. (4,5) peso 1.0
// 3. (7,8) peso 1.0
// 4. (1,4) peso 1.0
// 5. (6,7) peso 1.5
// 6. (5,8) peso 1.5
// 7. (3,4) peso 2.0 (conecta comp {3} com {0,1,4,5,7,8})
// 8. (2,5) peso 2.0 (conecta comp {2} com a árvore principal)
// Total de arestas = 8
// Peso total = 1.0 + 1.0 + 1.0 + 1.0 + 1.5 + 1.5 + 2.0 + 2.0 = 11.0
static void test_grid_3x3() {
  const char *name = "Grade 3x3 simulando imagem";
  TEST(name);

  std::vector<Edge> edges;
  edges.push_back(Edge(0, 1, 1.0));
  edges.push_back(Edge(1, 2, 10.0));
  edges.push_back(Edge(3, 4, 2.0));
  edges.push_back(Edge(4, 5, 1.0));
  edges.push_back(Edge(6, 7, 1.5));
  edges.push_back(Edge(7, 8, 1.0));
  edges.push_back(Edge(0, 3, 2.0));
  edges.push_back(Edge(1, 4, 1.0));
  edges.push_back(Edge(2, 5, 2.0));
  edges.push_back(Edge(3, 6, 10.0));
  edges.push_back(Edge(4, 7, 3.0));
  edges.push_back(Edge(5, 8, 1.5));

  MSTResult result = kruskal_mst(9, edges);

  assert(result.mst_edges.size() == 8);
  assert(approx(result.total_weight, 11.0));

  PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
  std::cout << "\n=== Testes do modulo Kruskal (MST) ===" << std::endl;

  test_linear_graph();
  test_triangle_cycle();
  test_grid_3x3();

  std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
            << tests_failed << " falharam ---\n"
            << std::endl;

  return tests_failed > 0 ? 1 : 0;
}

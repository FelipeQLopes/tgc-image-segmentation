// test_hierarchy.cpp — testes para o modulo Hierarchy (BPT / Dendrograma)
// Compilar:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/DisjointSet.cpp src/Kruskal.cpp
//   src/Hierarchy.cpp tests/test_hierarchy.cpp -o build/test_hierarchy

#include "hierarchy/Hierarchy.hpp"
#include "graph/Kruskal.hpp"
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
// 1. Grafo linear de 3 vertices: verifica estrutura da BPT
// ---------------------------------------------------------------------------
// 0 --1-- 1 --2-- 2
// MST: (0,1,1.0), (1,2,2.0)
// Hierarquia esperada:
//   No 3: merge de 0 e 1 no nivel 1.0 (size=2)
//   No 4: merge de (no 3) e 2 no nivel 2.0 (size=3)  <- raiz
static void test_linear_3_structure() {
    const char* name = "BPT de grafo linear 3 vertices — estrutura";
    TEST(name);

    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(0, 1, 1.0));
    mst_edges.push_back(Edge(1, 2, 2.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 3);

    // n=3 folhas, n-1=2 nos internos, total 5 nos
    assert(hier.nodes.size() == 5);
    assert(hier.root == 4);

    // Verificar nos folha
    for (int i = 0; i < 3; ++i) {
        assert(hier.nodes[i].is_leaf());
        assert(hier.nodes[i].size == 1);
    }

    // Verificar no interno 3: merge de 0 e 1
    const HierarchyNode& n3 = hier.nodes[3];
    assert(n3.id == 3);
    assert(!n3.is_leaf());
    assert(std::abs(n3.merge_level - 1.0) < 1e-9);
    assert(n3.size == 2);

    // Verificar no interno 4 (raiz): merge com 2
    const HierarchyNode& n4 = hier.nodes[4];
    assert(n4.id == 4);
    assert(!n4.is_leaf());
    assert(std::abs(n4.merge_level - 2.0) < 1e-9);
    assert(n4.size == 3);
    assert(n4.parent == -1); // raiz nao tem pai

    PASS(name);
}

// ---------------------------------------------------------------------------
// 2. cut_at_level(0) retorna cada pixel como segmento individual
// ---------------------------------------------------------------------------
static void test_cut_at_level_zero() {
    const char* name = "cut_at_level(0) — cada pixel e um segmento";
    TEST(name);

    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(0, 1, 1.0));
    mst_edges.push_back(Edge(1, 2, 2.0));
    mst_edges.push_back(Edge(2, 3, 3.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 4);

    std::vector<int> labels = hier.cut_at_level(0.0);
    assert(labels.size() == 4);

    // Cada pixel deve ter um label unico
    std::set<int> unique_labels(labels.begin(), labels.end());
    assert(unique_labels.size() == 4);

    assert(hier.num_segments_at_level(0.0) == 4);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 3. cut_at_level(valor_muito_alto) retorna um unico segmento
// ---------------------------------------------------------------------------
static void test_cut_at_level_high() {
    const char* name = "cut_at_level(1e9) — um unico segmento";
    TEST(name);

    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(0, 1, 1.0));
    mst_edges.push_back(Edge(1, 2, 2.0));
    mst_edges.push_back(Edge(2, 3, 3.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 4);

    std::vector<int> labels = hier.cut_at_level(1e9);
    assert(labels.size() == 4);

    // Todos os pixels devem ter o mesmo label
    std::set<int> unique_labels(labels.begin(), labels.end());
    assert(unique_labels.size() == 1);

    assert(hier.num_segments_at_level(1e9) == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Valores intermediarios de lambda — monotonia
// ---------------------------------------------------------------------------
// Mais segmentos com lambda baixo, menos com lambda alto
static void test_monotonicity() {
    const char* name = "Monotonia — mais segmentos com lambda baixo";
    TEST(name);

    // Grafo com 5 vertices, arestas com pesos 1, 2, 5, 10
    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(0, 1, 1.0));
    mst_edges.push_back(Edge(1, 2, 2.0));
    mst_edges.push_back(Edge(2, 3, 5.0));
    mst_edges.push_back(Edge(3, 4, 10.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 5);

    // Sequencia de lambdas crescentes
    double lambdas[] = {0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0, 7.0, 10.0, 100.0};
    int prev_segments = hier.num_segments_at_level(lambdas[0]);

    for (int i = 1; i < 10; ++i) {
        int cur_segments = hier.num_segments_at_level(lambdas[i]);
        assert(cur_segments <= prev_segments);
        prev_segments = cur_segments;
    }

    // Verificar valores especificos
    assert(hier.num_segments_at_level(0.0) == 5);  // nenhum merge
    assert(hier.num_segments_at_level(1.0) == 4);   // merge (0,1)
    assert(hier.num_segments_at_level(2.0) == 3);   // merge (0,1) e (1,2)
    assert(hier.num_segments_at_level(5.0) == 2);   // merge ate (2,3)
    assert(hier.num_segments_at_level(10.0) == 1);  // tudo junto

    PASS(name);
}

// ---------------------------------------------------------------------------
// 5. Grade 2x2 — verifica hierarquia com grafo de imagem
// ---------------------------------------------------------------------------
// Pixels:
//   0(=10) -- 1(=20)
//   |          |
//   2(=15) -- 3(=20)
//
// Pesos (4-vizinhanca):
//   (0,1) = |10-20| = 10
//   (0,2) = |10-15| = 5
//   (1,3) = |20-20| = 0
//   (2,3) = |15-20| = 5
//
// MST (ordenada): (1,3,0), (0,2,5), (2,3,5)
// Hierarquia:
//   No 4: merge 1 e 3, level=0, size=2
//   No 5: merge 0 e 2, level=5, size=2
//   No 6: merge no4 e no5, level=5, size=4 (raiz)
static void test_grid_2x2() {
    const char* name = "Grade 2x2 — hierarquia completa";
    TEST(name);

    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(1, 3, 0.0));
    mst_edges.push_back(Edge(0, 2, 5.0));
    mst_edges.push_back(Edge(2, 3, 5.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 4);

    // 4 folhas + 3 nos internos = 7 nos
    assert(hier.nodes.size() == 7);
    assert(hier.root == 6);

    // No 4: merge de 1 e 3 no nivel 0
    assert(std::abs(hier.nodes[4].merge_level - 0.0) < 1e-9);
    assert(hier.nodes[4].size == 2);

    // No 5: merge de 0 e 2 no nivel 5
    assert(std::abs(hier.nodes[5].merge_level - 5.0) < 1e-9);
    assert(hier.nodes[5].size == 2);

    // No 6: merge de no4 e no5 no nivel 5
    assert(std::abs(hier.nodes[6].merge_level - 5.0) < 1e-9);
    assert(hier.nodes[6].size == 4);

    // Cortes
    // lambda=0: merge (1,3) ja aconteceu, mas (0,2) e (2,3) nao
    // -> 3 segmentos: {0}, {1,3}, {2}
    assert(hier.num_segments_at_level(0.0) == 3);

    // lambda=5: tudo junto -> 1 segmento
    assert(hier.num_segments_at_level(5.0) == 1);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 6. Teste do LCA
// ---------------------------------------------------------------------------
static void test_lca() {
    const char* name = "LCA — menor ancestral comum";
    TEST(name);

    // Grafo linear: 0 --1-- 1 --2-- 2 --3-- 3
    std::vector<Edge> mst_edges;
    mst_edges.push_back(Edge(0, 1, 1.0));
    mst_edges.push_back(Edge(1, 2, 2.0));
    mst_edges.push_back(Edge(2, 3, 3.0));

    HierarchicalSegmentation hier;
    hier.build_from_mst(mst_edges, 4);

    // Hierarquia:
    //   No 4: merge(0,1), level=1
    //   No 5: merge(no4, 2), level=2
    //   No 6: merge(no5, 3), level=3 (raiz)

    // LCA(0,1) = 4 (merge direto)
    assert(hier.lca(0, 1) == 4);

    // LCA(0,2) = 5
    assert(hier.lca(0, 2) == 5);

    // LCA(0,3) = 6 (raiz)
    assert(hier.lca(0, 3) == 6);

    // LCA(2,3) = 6
    assert(hier.lca(2, 3) == 6);

    // LCA de um no com ele mesmo
    assert(hier.lca(0, 0) == 0);
    assert(hier.lca(2, 2) == 2);

    PASS(name);
}

// ---------------------------------------------------------------------------
// 7. N-1 nos internos para n folhas
// ---------------------------------------------------------------------------
static void test_internal_nodes_count() {
    const char* name = "Exatamente n-1 nos internos para n folhas";
    TEST(name);

    for (int n = 2; n <= 10; ++n) {
        std::vector<Edge> mst_edges;
        for (int i = 0; i < n - 1; ++i) {
            mst_edges.push_back(Edge(i, i + 1, static_cast<double>(i + 1)));
        }

        HierarchicalSegmentation hier;
        hier.build_from_mst(mst_edges, n);

        // Total de nos = 2n - 1
        assert(static_cast<int>(hier.nodes.size()) == 2 * n - 1);

        // Contar nos internos (nao-folha)
        int internal_count = 0;
        for (const auto& node : hier.nodes) {
            if (!node.is_leaf()) {
                ++internal_count;
            }
        }
        assert(internal_count == n - 1);
    }

    PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "\n=== Testes do modulo Hierarchy (BPT) ===" << std::endl;

    test_linear_3_structure();
    test_cut_at_level_zero();
    test_cut_at_level_high();
    test_monotonicity();
    test_grid_2x2();
    test_lca();
    test_internal_nodes_count();

    std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
              << tests_failed << " falharam ---\n"
              << std::endl;

    return tests_failed > 0 ? 1 : 0;
}

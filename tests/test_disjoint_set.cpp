// test_disjoint_set.cpp — testes para a estrutura DisjointSet
// Compilar:
//   g++ -std=c++17 -Wall -Wextra -Iinclude src/DisjointSet.cpp
//   tests/test_disjoint_set.cpp -o build/test_disjoint_set

#include "../include/DisjointSet.hpp"
#include <cassert>
#include <iostream>

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

// ---------------------------------------------------------------------------
// 1. Testa a Inicialização
// ---------------------------------------------------------------------------
static void test_initialization() {
  const char *name = "Inicialização do DisjointSet";
  TEST(name);

  int n = 10;
  DisjointSet ds(n);

  // Todos devem ser representantes de si mesmos no início
  for (int i = 0; i < n; ++i) {
    assert(ds.find(i) == i);
    assert(ds.component_size(i) == 1);
  }

  assert(ds.num_components() == n);

  PASS(name);
}

// ---------------------------------------------------------------------------
// 2. Testa União Básica e Encontro de Representantes
// ---------------------------------------------------------------------------
static void test_basic_union() {
  const char *name = "União básica (unite e find)";
  TEST(name);

  DisjointSet ds(5);

  // Une 0 e 1
  bool united1 = ds.unite(0, 1);
  assert(united1 == true);
  assert(ds.find(0) == ds.find(1));
  assert(ds.num_components() == 4);

  // Tentar unir 0 e 1 novamente deve retornar false e não alterar o count
  bool united2 = ds.unite(0, 1);
  assert(united2 == false);
  assert(ds.num_components() == 4);

  // Une 2 e 3
  ds.unite(2, 3);
  assert(ds.find(2) == ds.find(3));
  assert(ds.num_components() == 3);

  // Une as duas componentes independentes: {0, 1} e {2, 3}
  ds.unite(1, 2);
  assert(ds.find(0) == ds.find(3));
  assert(ds.num_components() == 2);

  // 4 permanece sozinho
  assert(ds.find(4) == 4);
  assert(ds.find(0) != ds.find(4));

  PASS(name);
}

// ---------------------------------------------------------------------------
// 3. Testa Tamanho dos Componentes
// ---------------------------------------------------------------------------
static void test_component_size() {
  const char *name = "Tamanho de componentes pós merges";
  TEST(name);

  DisjointSet ds(8);

  // União de pares
  ds.unite(0, 1); // Componente {0, 1}: tamanho 2
  ds.unite(2, 3); // Componente {2, 3}: tamanho 2
  ds.unite(4, 5); // Componente {4, 5}: tamanho 2
  ds.unite(6, 7); // Componente {6, 7}: tamanho 2

  for (int i = 0; i < 8; ++i) {
    assert(ds.component_size(i) == 2);
  }
  assert(ds.num_components() == 4);

  // Junta {0, 1} com {2, 3}
  ds.unite(0, 2);
  for (int i = 0; i < 4; ++i) {
    assert(ds.component_size(i) == 4);
  }
  for (int i = 4; i < 8; ++i) {
    assert(ds.component_size(i) == 2);
  }
  assert(ds.num_components() == 3);

  // Junta tudo na mesma componente
  ds.unite(0, 4);
  ds.unite(0, 6);
  for (int i = 0; i < 8; ++i) {
    assert(ds.component_size(i) == 8);
  }
  assert(ds.num_components() == 1);

  PASS(name);
}

// ---------------------------------------------------------------------------
// 4. Testa Union by Rank
// ---------------------------------------------------------------------------
static void test_union_by_rank() {
  const char *name = "União por rank";
  TEST(name);

  // Criaremos uma hierarquia específica para garantir que a união respeita o
  // rank
  DisjointSet ds(4);

  // 0 e 1 se unem. 0 se torna pai de 1 (ou vice-versa, dependendo do critério
  // de desempate, mas o rank aumenta).
  ds.unite(0, 1);
  int root_01 = ds.find(0);

  // 2 e 3 se unem. 2 se torna pai de 3.
  ds.unite(2, 3);
  int root_23 = ds.find(2);

  // Agora unimos os dois grupos. As duas raízes têm rank 1 (ou tamanho
  // similar). O rank de uma delas deve subir para 2.
  ds.unite(root_01, root_23);

  // Todos devem apontar para a mesma raiz
  int final_root = ds.find(0);
  assert(ds.find(1) == final_root);
  assert(ds.find(2) == final_root);
  assert(ds.find(3) == final_root);
  assert(ds.component_size(final_root) == 4);

  PASS(name);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
  std::cout << "\n=== Testes do modulo DisjointSet ===" << std::endl;

  test_initialization();
  test_basic_union();
  test_component_size();
  test_union_by_rank();

  std::cout << "\n--- Resultado: " << tests_passed << " passaram, "
            << tests_failed << " falharam ---\n"
            << std::endl;

  return tests_failed > 0 ? 1 : 0;
}

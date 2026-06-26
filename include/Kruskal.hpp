#ifndef KRUSKAL_HPP
#define KRUSKAL_HPP

#include "Edge.hpp"
#include <vector>

/**
 * @brief Estrutura contendo o resultado do cálculo da Árvore Geradora Mínima (MST).
 */
struct MSTResult {
  std::vector<Edge> mst_edges; // Vetor contendo as arestas selecionadas para a MST
  double total_weight;         // Somatório dos pesos de todas as arestas da MST
};

/**
 * @brief Calcula a Árvore Geradora Mínima (MST) de um grafo usando o algoritmo de Kruskal.
 * @param num_vertices Número total de vértices do grafo.
 * @param edges Lista de todas as arestas do grafo.
 * @return MSTResult contendo o conjunto de arestas da MST e seu peso acumulado.
 */
MSTResult kruskal_mst(int num_vertices, std::vector<Edge> edges);

#endif // KRUSKAL_HPP

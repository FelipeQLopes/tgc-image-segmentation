#include "../include/Kruskal.hpp"
#include "../include/DisjointSet.hpp"
#include <algorithm>

MSTResult kruskal_mst(int num_vertices, std::vector<Edge> edges) {
  MSTResult result;
  result.total_weight = 0.0;

  // Passo 1: Ordenar as arestas por peso crescente
  std::sort(edges.begin(), edges.end());

  // Passo 2: Inicializar o DSU com num_vertices elementos
  DisjointSet dsu(num_vertices);

  // Passo 3: Percorrer as arestas ordenadas
  size_t num_edges = edges.size();
  for (size_t i = 0; i < num_edges; ++i) {
    const Edge &edge = edges[i];
    // Se a MST já contiver n - 1 arestas, podemos parar prematuramente
    if (static_cast<int>(result.mst_edges.size()) == num_vertices - 1) {
      break;
    }

    int root_u = dsu.find(edge.u);
    int root_v = dsu.find(edge.v);

    // Se u e v pertencem a componentes diferentes, adiciona a aresta à MST
    if (root_u != root_v) {
      dsu.unite(root_u, root_v);
      result.mst_edges.push_back(edge);
      result.total_weight += edge.weight;
    }
  }

  return result;
}

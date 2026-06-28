#include "graph/DisjointSet.hpp"

DisjointSet::DisjointSet(int n) : parent(n), rank(n, 0), size(n, 1), count(n) {
  for (int i = 0; i < n; ++i) {
    parent[i] = i;
  }
}

int DisjointSet::find(int i) {
  if (parent[i] == i) {
    return i;
  }
  // Path compression: aponta diretamente para o representante raiz
  return parent[i] = find(parent[i]);
}

bool DisjointSet::unite(int i, int j) {
  int root_i = find(i);
  int root_j = find(j);

  if (root_i == root_j) {
    return false; // Já estão no mesmo conjunto
  }

  // Union by rank: une a árvore menor sob a raiz da árvore maior
  if (rank[root_i] < rank[root_j]) {
    parent[root_i] = root_j;
    size[root_j] += size[root_i];
  } else {
    parent[root_j] = root_i;
    size[root_i] += size[root_j];
    if (rank[root_i] == rank[root_j]) {
      rank[root_i]++;
    }
  }

  count--;
  return true;
}

int DisjointSet::component_size(int i) { return size[find(i)]; }

int DisjointSet::num_components() const { return count; }

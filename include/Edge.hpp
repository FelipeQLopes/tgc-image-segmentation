#ifndef EDGE_HPP
#define EDGE_HPP

/**
 * @brief Aresta ponderada do grafo de imagem.
 */
struct Edge {
    int    u;       // vertice de origem
    int    v;       // vertice de destino
    double weight;  // peso da aresta

    /** @brief Construtor padrao. */
    Edge() : u(0), v(0), weight(0.0) {}

    /**
     * @brief Construtor com valores.
     * @param u Vertice de origem.
     * @param v Vertice de destino.
     * @param w Peso da aresta.
     */
    Edge(int u, int v, double w) : u(u), v(v), weight(w) {}

    // Operador de comparacao por peso (para ordenacao)
    bool operator<(const Edge& other) const {
        return weight < other.weight;
    }
};

#endif // EDGE_HPP

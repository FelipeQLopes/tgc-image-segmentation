#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "Edge.hpp"
#include "image.hpp"

#include <algorithm>
#include <utility>
#include <vector>

// Tipo de vizinhanca
enum class Connectivity {
    FOUR,  // 4-vizinhanca (direita, baixo)
    EIGHT  // 8-vizinhanca (direita, baixo, diag-inf-dir, diag-inf-esq)
};

/**
 * @brief Grafo de imagem com arestas ponderadas.
 */
class ImageGraph {
public:
    int num_vertices;
    int width;
    int height;

    std::vector<Edge> edges;

    // Lista de adjacencia: adj[u] = { (v, peso), ... }
    std::vector<std::vector<std::pair<int, double>>> adj;

    /** @brief Construtor padrao. */
    ImageGraph();

    /**
     * @brief Constroi o grafo a partir de uma imagem e tipo de vizinhanca.
     * @param image Imagem de entrada (cinza ou RGB).
     * @param conn Tipo de vizinhanca (FOUR ou EIGHT).
     */
    ImageGraph(const Image& image, Connectivity conn);

    /**
     * @brief Converte coordenada (x, y) para id do vertice.
     * @param x Coluna do pixel.
     * @param y Linha do pixel.
     * @return Id do vertice (y * width + x).
     */
    int pixel_to_id(int x, int y) const;

    /**
     * @brief Converte id do vertice para coordenada (x, y).
     * @param id Id do vertice.
     * @param x Coluna do pixel (saida).
     * @param y Linha do pixel (saida).
     */
    void id_to_pixel(int id, int& x, int& y) const;

    /**
     * @brief Ordena as arestas por peso crescente (necessario para Kruskal).
     */
    void sort_edges();

private:
    /**
     * @brief Adiciona aresta entre dois pixels (lista de arestas + adjacencia).
     * @param u Vertice de origem.
     * @param v Vertice de destino.
     * @param weight Peso da aresta.
     */
    void add_edge(int u, int v, double weight);

    /**
     * @brief Calcula o peso da aresta entre dois pixels da imagem.
     * @param image Imagem de referencia.
     * @param x1 Coluna do primeiro pixel.
     * @param y1 Linha do primeiro pixel.
     * @param x2 Coluna do segundo pixel.
     * @param y2 Linha do segundo pixel.
     * @return Peso calculado.
     */
    static double compute_weight(const Image& image, int x1, int y1,
                                 int x2, int y2);
};

#endif // GRAPH_HPP

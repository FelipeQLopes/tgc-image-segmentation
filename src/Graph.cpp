#include "Graph.hpp"

#include <cmath>
#include <cstdlib>

/**
 * @brief Construtor padrao do ImageGraph.
 */
ImageGraph::ImageGraph()
    : num_vertices(0), width(0), height(0) {}

/**
 * @brief Constroi o grafo a partir de uma imagem e tipo de vizinhanca.
 * @param image Imagem de entrada (cinza ou RGB).
 * @param conn Tipo de vizinhanca (FOUR ou EIGHT).
 */
ImageGraph::ImageGraph(const Image& image, Connectivity conn)
    : num_vertices(image.width * image.height),
      width(image.width),
      height(image.height),
      adj(static_cast<size_t>(num_vertices))
{
    // Percorre cada pixel e adiciona arestas apenas para vizinhos "a frente"
    // (direita, baixo, diagonais inferiores) para evitar duplicatas.

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Direita
            if (x + 1 < width) {
                double w = compute_weight(image, x, y, x + 1, y);
                add_edge(pixel_to_id(x, y), pixel_to_id(x + 1, y), w);
            }

            // Baixo
            if (y + 1 < height) {
                double w = compute_weight(image, x, y, x, y + 1);
                add_edge(pixel_to_id(x, y), pixel_to_id(x, y + 1), w);
            }

            if (conn == Connectivity::EIGHT) {
                // Diagonal inferior direita
                if (x + 1 < width && y + 1 < height) {
                    double w = compute_weight(image, x, y, x + 1, y + 1);
                    add_edge(pixel_to_id(x, y), pixel_to_id(x + 1, y + 1), w);
                }

                // Diagonal inferior esquerda
                if (x - 1 >= 0 && y + 1 < height) {
                    double w = compute_weight(image, x, y, x - 1, y + 1);
                    add_edge(pixel_to_id(x, y), pixel_to_id(x - 1, y + 1), w);
                }
            }
        }
    }
}

/**
 * @brief Converte coordenada (x, y) para id do vertice.
 * @param x Coluna do pixel.
 * @param y Linha do pixel.
 * @return Id do vertice (y * width + x).
 */
int ImageGraph::pixel_to_id(int x, int y) const {
    return y * width + x;
}

/**
 * @brief Converte id do vertice para coordenada (x, y).
 * @param id Id do vertice.
 * @param x Coluna do pixel (saida).
 * @param y Linha do pixel (saida).
 */
void ImageGraph::id_to_pixel(int id, int& x, int& y) const {
    x = id % width;
    y = id / width;
}

/**
 * @brief Ordena as arestas por peso crescente (necessario para Kruskal).
 */
void ImageGraph::sort_edges() {
    std::sort(edges.begin(), edges.end());
}

/**
 * @brief Adiciona aresta ao grafo (lista de arestas + lista de adjacencia).
 * @param u Vertice de origem.
 * @param v Vertice de destino.
 * @param weight Peso da aresta.
 */
void ImageGraph::add_edge(int u, int v, double weight) {
    edges.emplace_back(u, v, weight);
    adj[u].emplace_back(v, weight);
    adj[v].emplace_back(u, weight);
}

/**
 * @brief Calcula o peso da aresta entre dois pixels da imagem.
 * @param image Imagem de referencia.
 * @param x1 Coluna do primeiro pixel.
 * @param y1 Linha do primeiro pixel.
 * @param x2 Coluna do segundo pixel.
 * @param y2 Linha do segundo pixel.
 * @return Peso: |i1 - i2| para cinza, distancia euclidiana RGB para colorida.
 */
double ImageGraph::compute_weight(const Image& image,
                                  int x1, int y1,
                                  int x2, int y2) {
    if (image.channels == 1) {
        // Cinza: peso = |intensidade_1 - intensidade_2|
        return std::abs(static_cast<double>(image.at(x1, y1, 0)) -
                        static_cast<double>(image.at(x2, y2, 0)));
    } else {
        // Colorida: peso = sqrt((r1-r2)^2 + (g1-g2)^2 + (b1-b2)^2)
        double dr = static_cast<double>(image.at(x1, y1, 0)) -
                    static_cast<double>(image.at(x2, y2, 0));
        double dg = static_cast<double>(image.at(x1, y1, 1)) -
                    static_cast<double>(image.at(x2, y2, 1));
        double db = static_cast<double>(image.at(x1, y1, 2)) -
                    static_cast<double>(image.at(x2, y2, 2));
        return std::sqrt(dr * dr + dg * dg + db * db);
    }
}

#include "Graph.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace {

int clamp_to_pixel_range(int value, int min_value, int max_value) {
    return std::max(min_value, std::min(max_value, value));
}

double compute_gray_gradient(const Image& image, int x, int y) {
    const int center = image.at(x, y, 0);

    int left = center;
    int right = center;
    int up = center;
    int down = center;

    if (x > 0) left = image.at(x - 1, y, 0);
    if (x + 1 < image.width) right = image.at(x + 1, y, 0);
    if (y > 0) up = image.at(x, y - 1, 0);
    if (y + 1 < image.height) down = image.at(x, y + 1, 0);

    return std::max({
        std::abs(static_cast<double>(center) - static_cast<double>(left)),
        std::abs(static_cast<double>(center) - static_cast<double>(right)),
        std::abs(static_cast<double>(center) - static_cast<double>(up)),
        std::abs(static_cast<double>(center) - static_cast<double>(down))
    });
}

double compute_rgb_gradient(const Image& image, int x, int y) {
    const int channels = image.channels;
    std::array<double, 3> center = {
        static_cast<double>(image.at(x, y, 0)),
        static_cast<double>(image.at(x, y, 1)),
        static_cast<double>(image.at(x, y, 2))
    };

    std::array<double, 3> max_diff = {{0.0, 0.0, 0.0}};
    const std::array<int, 4> dx = {{-1, 1, 0, 0}};
    const std::array<int, 4> dy = {{0, 0, -1, 1}};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx < 0 || ny < 0 || nx >= image.width || ny >= image.height) {
            continue;
        }

        for (int c = 0; c < channels; ++c) {
            double diff = std::abs(center[c] - static_cast<double>(image.at(nx, ny, c)));
            max_diff[c] = std::max(max_diff[c], diff);
        }
    }

    double grad = 0.0;
    for (int c = 0; c < channels; ++c) {
        grad += max_diff[c] * max_diff[c];
    }
    return std::sqrt(grad);
}

} // namespace

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
        double diff = std::abs(static_cast<double>(image.at(x1, y1, 0)) -
                               static_cast<double>(image.at(x2, y2, 0)));
        double grad = std::max(compute_gray_gradient(image, x1, y1),
                               compute_gray_gradient(image, x2, y2));
        return std::min(255.0, diff * 1.8 + 0.7 * grad);
    } else {
        double dr = static_cast<double>(image.at(x1, y1, 0)) -
                    static_cast<double>(image.at(x2, y2, 0));
        double dg = static_cast<double>(image.at(x1, y1, 1)) -
                    static_cast<double>(image.at(x2, y2, 1));
        double db = static_cast<double>(image.at(x1, y1, 2)) -
                    static_cast<double>(image.at(x2, y2, 2));
        double color_dist = std::sqrt(dr * dr + dg * dg + db * db);
        double grad = std::max(compute_rgb_gradient(image, x1, y1),
                               compute_rgb_gradient(image, x2, y2));
        
        // Usa a mesma escala da versao em tons de cinza (1.8 e 0.7)
        // para que o limiar lambda funcione da mesma forma para imagens coloridas.
        return std::min(255.0, color_dist * 1.8 + 0.7 * grad);
    }
}

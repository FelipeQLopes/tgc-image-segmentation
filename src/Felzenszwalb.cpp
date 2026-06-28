#include "Felzenszwalb.hpp"
#include "DisjointSet.hpp"
#include "Graph.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <set>
#include <stdexcept>
#include <vector>

namespace {

/**
 * @brief Aplica um blur gaussiano simples na imagem.
 * @param image Imagem de entrada.
 * @param sigma Desvio padrao do filtro.
 * @return Copia da imagem com o filtro aplicado.
 */
Image gaussian_blur(const Image& image, double sigma) {
    if (image.channels != 1 && image.channels != 3) {
        throw std::runtime_error("Felzenszwalb: imagem deve ter 1 ou 3 canais");
    }
    if (sigma <= 0.0) {
        return image;
    }

    const int radius = std::max(1, static_cast<int>(std::ceil(2.0 * sigma)));
    const int size = 2 * radius + 1;
    std::vector<double> kernel(size);
    double sum = 0.0;
    for (int i = 0; i < size; ++i) {
        int x = i - radius;
        double value = std::exp(-(x * x) / (2.0 * sigma * sigma));
        kernel[i] = value;
        sum += value;
    }
    for (double& value : kernel) {
        value /= sum;
    }

    Image blurred(image.width, image.height, image.channels);
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            for (int c = 0; c < image.channels; ++c) {
                double acc = 0.0;
                for (int i = 0; i < size; ++i) {
                    int nx = x + (i - radius);
                    if (nx < 0 || nx >= image.width) {
                        continue;
                    }
                    acc += static_cast<double>(image.at(nx, y, c)) * kernel[i];
                }
                blurred.set(x, y, c, static_cast<uint8_t>(std::max(0.0, std::min(255.0, acc))));
            }
        }
    }
    return blurred;
}

/**
 * @brief Renumeriza os labels para valores sequenciais.
 * @param labels Vetor de labels originais.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return Vetor de labels renumerizados.
 */
std::vector<int> renumber_labels(const std::vector<int>& labels,
                                 int width, int height) {
    std::vector<int> remapped(width * height, -1);
    std::vector<int> mapping;
    std::set<int> seen;
    for (int i = 0; i < width * height; ++i) {
        int label = labels[i];
        auto it = seen.find(label);
        if (it == seen.end()) {
            seen.insert(label);
            mapping.push_back(label);
        }
    }
    for (int i = 0; i < width * height; ++i) {
        int label = labels[i];
        for (size_t j = 0; j < mapping.size(); ++j) {
            if (mapping[j] == label) {
                remapped[i] = static_cast<int>(j);
                break;
            }
        }
    }
    return remapped;
}

Image build_boundary_saliency(const std::vector<int>& labels,
                              int width,
                              int height,
                              const std::vector<Edge>& edges) {
    Image saliency(width, height, 1);
    if (edges.empty()) {
        return saliency;
    }

    double max_weight = 0.0;
    for (const Edge& edge : edges) {
        max_weight = std::max(max_weight, edge.weight);
    }

    std::vector<double> strengths(static_cast<size_t>(width * height), 0.0);
    for (const Edge& edge : edges) {
        if (labels[edge.u] == labels[edge.v]) {
            continue;
        }

        double strength = 0.0;
        if (max_weight > 0.0) {
            strength = (edge.weight / max_weight) * 255.0;
        }

        strengths[edge.u] = std::max(strengths[edge.u], strength);
        strengths[edge.v] = std::max(strengths[edge.v], strength);
    }

    for (int i = 0; i < width * height; ++i) {
        int value = static_cast<int>(std::round(strengths[i]));
        if (value < 0) value = 0;
        if (value > 255) value = 255;
        saliency.set(i % width, i / width, 0, static_cast<uint8_t>(value));
    }

    return saliency;
}

}  // namespace

SegmentationResult felzenszwalb_segment(const Image& image,
                                         const FelzenszwalbParams& params) {
    auto t_start = std::chrono::high_resolution_clock::now();

    if (params.connectivity != 4 && params.connectivity != 8) {
        throw std::invalid_argument("Felzenszwalb: connectivity deve ser 4 ou 8");
    }
    if (image.channels != 1 && image.channels != 3) {
        throw std::runtime_error("Felzenszwalb: imagem deve ter 1 ou 3 canais");
    }

    Image preprocessed = gaussian_blur(image, params.sigma);
    Connectivity conn = (params.connectivity == 4)
                            ? Connectivity::FOUR
                            : Connectivity::EIGHT;
    ImageGraph graph(preprocessed, conn);

    graph.sort_edges();

    DisjointSet dsu(static_cast<int>(graph.num_vertices));
    std::vector<double> internal_diff(graph.num_vertices, 0.0);

    for (const Edge& edge : graph.edges) {
        int cu = dsu.find(edge.u);
        int cv = dsu.find(edge.v);
        if (cu == cv) {
            continue;
        }

        double tau_u = params.k / static_cast<double>(dsu.component_size(cu));
        double tau_v = params.k / static_cast<double>(dsu.component_size(cv));
        double threshold = std::min(internal_diff[cu] + tau_u,
                                    internal_diff[cv] + tau_v);

        if (edge.weight <= threshold) {
            dsu.unite(cu, cv);
            int root = dsu.find(cu);
            internal_diff[root] = edge.weight;
        }
    }

    for (const Edge& edge : graph.edges) {
        int ru = dsu.find(edge.u);
        int rv = dsu.find(edge.v);
        if (ru == rv) {
            continue;
        }

        if (dsu.component_size(ru) < params.min_size ||
            dsu.component_size(rv) < params.min_size) {
            dsu.unite(ru, rv);
        }
    }

    std::vector<int> labels(graph.num_vertices);
    for (int i = 0; i < graph.num_vertices; ++i) {
        labels[i] = dsu.find(i);
    }

    labels = renumber_labels(labels, image.width, image.height);

    SegmentationResult result;
    result.num_segments =
        static_cast<int>(std::set<int>(labels.begin(), labels.end()).size());
    if (result.num_segments == 0) {
        result.num_segments = 1;
    }

    if (params.compute_saliency) {
        result.saliency_image = build_boundary_saliency(
            labels, image.width, image.height, graph.edges);
    }

    result.segmentation_image =
        colorize_segmentation(labels, image.width, image.height);

    auto t_end = std::chrono::high_resolution_clock::now();
    result.elapsed_ms = std::chrono::duration<double, std::milli>(
                            t_end - t_start)
                            .count();

    return result;
}

#include "IFT.hpp"
#include "Gradient.hpp"
#include "Cousty.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace {

struct IFTNode {
    double cost;
    int node;
    bool operator>(const IFTNode& other) const {
        return cost > other.cost;
    }
};

int pixel_to_id(int x, int y, int width) {
    return y * width + x;
}

void id_to_pixel(int id, int width, int& x, int& y) {
    x = id % width;
    y = id / width;
}

std::vector<std::pair<int, double>> build_neighbors(int node, int width, int height, int connectivity,
                                                    const Image& gradient) {
    std::vector<std::pair<int, double>> neighbors;
    int x = node % width;
    int y = node / width;

    static const std::vector<std::pair<int, int>> dirs = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
    };

    int count = (connectivity == 4) ? 4 : 8;
    for (int i = 0; i < count; ++i) {
        int nx = x + dirs[i].first;
        int ny = y + dirs[i].second;
        if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
        int neighbor_id = pixel_to_id(nx, ny, width);
        double w = std::max(static_cast<double>(gradient.at(x, y, 0)),
                            static_cast<double>(gradient.at(nx, ny, 0)));
        neighbors.emplace_back(neighbor_id, w);
    }
    return neighbors;
}

std::vector<int> normalize_seed_labels(const std::vector<int>& input, int width, int height) {
    std::vector<int> labels(width * height, -1);
    std::unordered_map<int, int> remap;
    int next = 0;
    for (int i = 0; i < width * height; ++i) {
        if (input[i] < 0) continue;
        auto it = remap.find(input[i]);
        if (it == remap.end()) {
            remap[input[i]] = next++;
        }
        labels[i] = remap[input[i]];
    }
    return labels;
}

}  // namespace

SegmentationResult ift_segment(const Image& image, const IFTParams& params) {
    if (params.connectivity != 4 && params.connectivity != 8) {
        throw std::invalid_argument("IFT: connectivity deve ser 4 ou 8");
    }

    if (!params.auto_seeds) {
        return ift_segment_with_seeds(image, {}, params.connectivity);
    }

    Image gradient = compute_gradient(image);
    std::vector<int> seeds = find_regional_minima(gradient);
    return ift_segment_with_seeds(image, seeds, params.connectivity);
}

SegmentationResult ift_segment_with_seeds(
    const Image& image,
    const std::vector<int>& seed_labels,
    int connectivity
) {
    auto t_start = std::chrono::high_resolution_clock::now();

    const int width = image.width;
    const int height = image.height;
    const int n_pixels = width * height;

    std::vector<IFTStatus> status(n_pixels, IFTStatus::WHITE);

    Image gradient = compute_gradient(image);

    std::vector<int> seeds = seed_labels;
    if (seeds.empty()) {
        seeds = find_regional_minima(gradient);
    }

    std::vector<int> normalized = normalize_seed_labels(seeds, width, height);

    std::priority_queue<IFTNode, std::vector<IFTNode>, std::greater<IFTNode>> pq;

    std::vector<double> cost(n_pixels, std::numeric_limits<double>::infinity());
    std::vector<int> labels(n_pixels, -1);

    for (int i = 0; i < n_pixels; ++i) {
        if (normalized[i] >= 0) {
            labels[i] = normalized[i];
            cost[i] = 0.0;
            pq.push({0.0, i});
        }
    }

    while (!pq.empty()) {
        IFTNode node = pq.top(); pq.pop();
        int u = node.node;

        if (status[u] == IFTStatus::BLACK) continue;
        status[u] = IFTStatus::BLACK;

        auto neighbors = build_neighbors(u, width, height, connectivity, gradient);

        for (const auto& [v, w] : neighbors) {
            if (status[v] == IFTStatus::BLACK) continue;

            double new_cost = std::max(cost[u], w);

            if (new_cost < cost[v]) {
                cost[v] = new_cost;
                labels[v] = labels[u];
                pq.push({new_cost, v});
            }
        }
    }

    SegmentationResult result;
    result.num_segments = static_cast<int>(
        std::set<int>(labels.begin(), labels.end()).size()
    );

    if (result.num_segments == 0) {
        result.num_segments = 1;
    }

    result.segmentation_image =
        colorize_segmentation(labels, width, height);

    auto t_end = std::chrono::high_resolution_clock::now();
    result.elapsed_ms =
        std::chrono::duration<double, std::milli>(t_end - t_start).count();

    return result;
}
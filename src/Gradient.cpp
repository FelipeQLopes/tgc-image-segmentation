#include "Gradient.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <iostream>

// Helper: clamp para 0-255
static uint8_t clamp255(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return static_cast<uint8_t>(v);
}

Image compute_gradient(const Image& image) {
    if (image.channels != 1 && image.channels != 3) {
        throw std::runtime_error("compute_gradient: imagem deve ter 1 ou 3 canais");
    }

    int w = image.width;
    int h = image.height;
    Image grad(w, h, 1);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int max_chan_diff = 0;

            //para cada canal (1 ou 3) e pegar o maximo entre os canais
            int nch = image.channels;
            for (int c = 0; c < nch; ++c) {
                int minv = 255;
                int maxv = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                        int v = image.at(nx, ny, c);
                        if (v < minv) minv = v;
                        if (v > maxv) maxv = v;
                    }
                }
                int diff = maxv - minv;
                if (diff > max_chan_diff) max_chan_diff = diff;
            }

            grad.set(x, y, 0, clamp255(max_chan_diff));
        }
    }

    return grad;
}

std::vector<RegionalMinimum> extract_regional_minima(const Image& gradient) {
    if (gradient.channels != 1) {
        throw std::runtime_error("find_regional_minima: gradient deve ter 1 canal");
    }

    int w = gradient.width;
    int h = gradient.height;

    std::vector<char> visited(w * h, 0);
    std::vector<RegionalMinimum> minima;

    auto id = [&](int x, int y){ return y * w + x; };

    int current_label = 0;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {

            int idx = id(x, y);
            if (visited[idx]) continue;

            int val = gradient.at(x, y, 0);

            std::queue<int> q;
            std::vector<int> comp;

            visited[idx] = 1;
            q.push(idx);
            comp.push_back(idx);

            bool is_min = true;

            while (!q.empty()) {

                int cur = q.front();
                q.pop();

                int cx = cur % w;
                int cy = cur / w;

                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {

                        if (dx == 0 && dy == 0)
                            continue;

                        int nx = cx + dx;
                        int ny = cy + dy;

                        if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                            continue;

                        int nidx = id(nx, ny);
                        int nval = gradient.at(nx, ny, 0);

                        if (nval == val) {

                            if (!visited[nidx]) {
                                visited[nidx] = 1;
                                q.push(nidx);
                                comp.push_back(nidx);
                            }

                        } else {

                            if (nval <= val)
                                is_min = false;
                        }
                    }
                }
            }

            if (is_min) {

                RegionalMinimum rm;
                rm.label = current_label;
                rm.level = val;
                rm.pixels = std::move(comp);

                minima.push_back(std::move(rm));

                current_label++;
            }
        }
    }

    return minima;
}
#ifndef MARKER_GENERATION_HPP
#define MARKER_GENERATION_HPP

#include "image.hpp"
#include "Gradient.hpp"
#include <vector>

struct MarkerParams {
    int gaussian_radius = 2;       // opcional
    int min_region_size = 10;      // elimina mínimos pequenos
    int gradient_threshold = 20;   // elimina mínimos fracos
};



std::vector<int> generate_markers(
    const Image& image,
    const MarkerParams& params);

#endif
#include "image_processing/MarkerGeneration.hpp"
#include "image_processing/Gradient.hpp"

std::vector<int> generate_markers(
    const Image& image,
    const MarkerParams& params)
{
    Image gradient = compute_gradient(image);

    auto minima = extract_regional_minima(gradient);

    std::vector<int> labels(
        image.width * image.height,
        -1);

    int current_label = 0;

    for (const auto& m : minima)
    {
        // Remove mínimos muito pequenos
        if ((int)m.pixels.size() < params.min_region_size)
            continue;

        // Remove mínimos em regiões de gradiente elevado
        // if (m.level > params.gradient_threshold)
        //     continue;

        for (int p : m.pixels)
            labels[p] = current_label;

        current_label++;
    }

    return labels;
}
#pragma once

#include "core/Image.hpp"

#include <vector>

class GaussianBlur
{
public:

    static std::vector<float> build_kernel(float sigma);

    static int kernel_radius(float sigma);

    static Image apply(const Image& src, float sigma);

private:

    static void convolve_horizontal(
        const Image& src,
        Image& dst,
        const std::vector<float>& kernel,
        int ch);

    static void convolve_vertical(
        const Image& src,
        Image& dst,
        const std::vector<float>& kernel,
        int ch);
};
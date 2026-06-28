#include "Felzenszwalb.hpp"

#include <cassert>
#include <iostream>

/**
 * @brief Testa a segmentacao em imagem cinza sintetica.
 */
static void test_gray_synthetic_image() {
    Image img(16, 16, 1);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            bool inside_square = x >= 4 && x <= 11 && y >= 4 && y <= 11;
            img.set(x, y, 0, inside_square ? 255 : 0);
        }
    }

    FelzenszwalbParams params;
    params.k = 100.0;
    params.min_size = 4;
    params.connectivity = 8;
    params.sigma = 0.5;

    SegmentationResult result = felzenszwalb_segment(img, params);
    assert(result.segmentation_image.width == img.width);
    assert(result.segmentation_image.height == img.height);
    assert(result.saliency_image.width == img.width);
    assert(result.saliency_image.height == img.height);
    assert(result.saliency_image.channels == 1);
    assert(result.num_segments >= 2);
    assert(result.elapsed_ms >= 0.0);
}

/**
 * @brief Testa a segmentacao em imagem RGB sintetica.
 */
static void test_rgb_image() {
    Image img(12, 12, 3);
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            bool inside_square = x >= 3 && x <= 8 && y >= 3 && y <= 8;
            uint8_t value = inside_square ? 255 : 20;
            img.set(x, y, 0, value);
            img.set(x, y, 1, value);
            img.set(x, y, 2, value);
        }
    }

    FelzenszwalbParams params;
    params.k = 80.0;
    params.min_size = 3;
    params.connectivity = 4;
    params.sigma = 0.5;

    SegmentationResult result = felzenszwalb_segment(img, params);
    assert(result.num_segments >= 2);
    assert(result.segmentation_image.channels == 3);
}

int main() {
    std::cout << "[test_felzenszwalb]" << std::endl;
    test_gray_synthetic_image();
    test_rgb_image();
    std::cout << "Todos os testes de Felzenszwalb passaram" << std::endl;
    return 0;
}

#include "algorithms/IFT.hpp"

#include "core/Image.hpp"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

static int tests_passed = 0;

#define TEST(name)                                                            \
    do {                                                                      \
        std::cout << "  [RUN ] " << (name) << std::endl;                      \
    } while (0)

#define PASS(name)                                                            \
    do {                                                                      \
        std::cout << "  [PASS] " << (name) << std::endl;                      \
        ++tests_passed;                                                      \
    } while (0)

static void test_ift_on_synthetic_image() {
    const char* name = "IFT em imagem sintetica";
    TEST(name);

    Image img(8, 8, 1);
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            img.set(x, y, 0, 0);
        }
    }

    for (int y = 2; y < 6; ++y) {
        for (int x = 2; x < 6; ++x) {
            img.set(x, y, 0, 200);
        }
    }

    IFTParams params;
    params.connectivity = 4;
    params.auto_seeds = true;

    IFTResult result = ift_segment(img, params);

    assert(result.labels.size() == static_cast<size_t>(img.width * img.height));
    assert(result.costs.size() == static_cast<size_t>(img.width * img.height));
    assert(result.predecessors.size() == static_cast<size_t>(img.width * img.height));
    assert(result.num_segments >= 2);
    assert(result.elapsed_ms >= 0.0);

    PASS(name);
}

static void test_ift_reads_test_data_and_saves_results() {
    const char* name = "IFT lê imagem de tests/data e salva em results";
    TEST(name);

    std::filesystem::create_directories("results");

    std::string input_path = "tests/data/visual_gray_step.png";
    Image img = load_image(input_path);

    IFTParams params;
    params.connectivity = 4;
    params.auto_seeds = true;

    IFTResult result = ift_segment(img, params);
    save_image("results/ift_visual_gray_step.png", result.segmentation_image);

    assert(std::filesystem::exists("results/ift_visual_gray_step.png"));
    PASS(name);
}

int main() {
    test_ift_on_synthetic_image();
    test_ift_reads_test_data_and_saves_results();

    std::cout << "\nTests passed: " << tests_passed << "\n";
    return 0;
}

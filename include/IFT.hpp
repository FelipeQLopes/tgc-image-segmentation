#ifndef IFT_HPP
#define IFT_HPP

#include "image.hpp"
#include "SegmentationResult.hpp"

#include <limits>
#include <string>
#include <vector>

struct IFTParams {
    int connectivity = 4;  // 4 ou 8
    bool auto_seeds = true;
};

// struct IFTResult {
//     std::vector<int> labels;
//     std::vector<double> costs;
//     std::vector<int> predecessors;
//     int num_segments = 0;
//     double elapsed_ms = 0.0;
//     Image segmentation_image;
// };

enum class IFTStatus {
    WHITE,
    BLACK
};

//SegmentationResult convert(const IFTResult& r)

SegmentationResult ift_segment(const Image& image, const IFTParams& params);
SegmentationResult ift_segment_with_seeds(const Image& image,
                                          const std::vector<int>& seed_labels,
                                          int connectivity);
#endif // IFT_HPP

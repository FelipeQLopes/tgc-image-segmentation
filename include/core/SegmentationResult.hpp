#ifndef SEGMENTATION_RESULT_HPP
#define SEGMENTATION_RESULT_HPP

#include "core/Image.hpp"

struct SegmentationResult {
    Image segmentation_image;
    int num_segments = 0;
    double elapsed_ms = 0.0;
    Image saliency_image;
};

#endif
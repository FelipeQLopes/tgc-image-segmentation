#ifndef SEGMENTATION_RESULT_HPP
#define SEGMENTATION_RESULT_HPP

#include "image.hpp"

struct SegmentationResult {
    Image segmentation_image;
    int num_segments;
    double elapsed_ms;
    Image saliency_image;
};

#endif
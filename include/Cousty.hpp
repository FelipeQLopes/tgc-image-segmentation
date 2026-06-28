#ifndef COUSTY_HPP
#define COUSTY_HPP

#include "image.hpp"
#include "SegmentationResult.hpp"

#include <string>
#include <vector>

struct CoustyParams {
    double lambda;
    int connectivity;
    bool compute_saliency;

    CoustyParams()
        : lambda(30.0), connectivity(8), compute_saliency(true) {}
};

SegmentationResult cousty_segment(const Image& image, const CoustyParams& params);

#endif
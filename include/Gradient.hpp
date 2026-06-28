#ifndef GRADIENT_HPP
#define GRADIENT_HPP

#include "image.hpp"

#include <vector>

Image compute_gradient(const Image& image);

// Estrutura com informações completas de um mínimo regional
struct RegionalMinimum {
    int label;
    int level;
    std::vector<int> pixels;
};

// Nova função
std::vector<RegionalMinimum>
extract_regional_minima(const Image& gradient);

// Mantém a função antiga para compatibilidade
std::vector<int>
find_regional_minima(const Image& gradient);

#endif
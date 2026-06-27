#ifndef GRADIENT_HPP
#define GRADIENT_HPP

#include "image.hpp"

#include <vector>

// Calcula o mapa de gradiente da imagem e retorna uma imagem de 1 canal.
Image compute_gradient(const Image& image);

// Encontra minimos regionais no mapa de gradiente (deve ser 1 canal).
// Retorna um vetor de labels com tamanho width*height, onde cada minimo
// regional recebe um label unico (0..k-1) e pixels nao-minimo recebem -1.
std::vector<int> find_regional_minima(const Image& gradient);

#endif // GRADIENT_HPP

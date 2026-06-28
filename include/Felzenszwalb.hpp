#ifndef FELZENSZWALB_HPP
#define FELZENSZWALB_HPP

#include "image.hpp"
#include "SegmentationResult.hpp"

/**
 * @brief Estrutura de parametros para o algoritmo de Felzenszwalb-Huttenlocher.
 */
struct FelzenszwalbParams {
    double k;          // parametro de escala
    int min_size;      // tamanho minimo de componente
    int connectivity;  // 4 ou 8
    double sigma;      // desvio padrao do Gaussiano
    bool compute_saliency; // se true, gera o mapa de saliencia

    FelzenszwalbParams()
        : k(300.0), min_size(50), connectivity(8), sigma(0.5), compute_saliency(true) {}
};

/**
 * @brief Segmenta uma imagem com o algoritmo de Felzenszwalb-Huttenlocher.
 * @param image Imagem de entrada (cinza ou RGB).
 * @param params Parametros do algoritmo.
 * @return SegmentationResult com labels, numero de segmentos e tempo de execucao.
 */
SegmentationResult felzenszwalb_segment(const Image& image,
                                         const FelzenszwalbParams& params);

#endif

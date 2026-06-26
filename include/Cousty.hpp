#ifndef COUSTY_HPP
#define COUSTY_HPP

#include "image.hpp"

#include <string>
#include <vector>

/**
 * @brief Parametros para o pipeline de segmentacao de Cousty.
 */
struct CoustyParams {
    double lambda;           // nivel de corte na hierarquia (default: 30.0)
    int    connectivity;     // 4 ou 8 (default: 8)
    bool   compute_saliency; // se deve gerar saliency map (default: true)

    CoustyParams()
        : lambda(8.0), connectivity(8), compute_saliency(true) {}
};

/**
 * @brief Resultado da segmentacao hierarquica de Cousty.
 */
struct SegmentationResult {
    std::vector<int> labels;       // label de cada pixel
    int              num_segments; // numero total de segmentos
    double           elapsed_ms;   // tempo de execucao em milissegundos
    Image            segmentation_image; // imagem colorizada da segmentacao
    Image            saliency_image;     // saliency map (vazio se nao calculado)
};

/**
 * @brief Executa o pipeline completo de segmentacao hierarquica de Cousty.
 *
 * Etapas:
 * 1. Constroi grafo da imagem (4 ou 8-vizinhanca).
 * 2. Calcula MST com Kruskal.
 * 3. Constroi hierarquia (BPT) a partir da MST.
 * 4. Corta hierarquia no nivel lambda.
 * 5. (Opcional) Gera saliency map.
 * 6. Gera imagem colorizada da segmentacao.
 * 7. Mede tempo de execucao total.
 *
 * @param image Imagem de entrada (cinza ou RGB).
 * @param params Parametros do pipeline.
 * @return SegmentationResult com labels, num_segments, tempo e imagens.
 */
SegmentationResult cousty_segment(const Image& image, const CoustyParams& params);

#endif // COUSTY_HPP

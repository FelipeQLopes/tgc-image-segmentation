#ifndef SALIENCY_MAP_HPP
#define SALIENCY_MAP_HPP

#include "graph/Graph.hpp"
#include "hierarchy/Hierarchy.hpp"
#include "core/Image.hpp"

/**
 * @brief Gera o mapa de saliencia a partir da hierarquia e do grafo original.
 *
 * Para cada aresta (u, v) do grafo original, o valor de saliencia e o
 * merge_level do menor ancestral comum (LCA) de u e v na hierarquia.
 * Os valores sao normalizados para o intervalo 0-255.
 *
 * @param hierarchy Hierarquia construida a partir da MST.
 * @param graph Grafo original da imagem (contem todas as arestas).
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return Imagem de 1 canal (cinza) com valores de saliencia normalizados.
 */
Image compute_saliency_map(const HierarchicalSegmentation& hierarchy,
                           const ImageGraph& graph,
                           int width, int height);

#endif // SALIENCY_MAP_HPP

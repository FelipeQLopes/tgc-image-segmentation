#include "hierarchy/SaliencyMap.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

/**
 * @brief Gera o mapa de saliencia a partir da hierarquia e do grafo original.
 *
 * Para cada aresta (u, v) do grafo original:
 * - Encontra o LCA de u e v na hierarquia.
 * - O valor de saliencia e o merge_level desse LCA.
 * - Marca o pixel de fronteira correspondente na imagem de saida.
 * - Normaliza para 0-255.
 *
 * @param hierarchy Hierarquia construida a partir da MST.
 * @param graph Grafo original da imagem.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return Imagem de 1 canal com saliencia normalizada.
 */
Image compute_saliency_map(const HierarchicalSegmentation& hierarchy,
                           const ImageGraph& graph,
                           int width, int height) {
    // Imagem de saliencia em double (acumula maximos antes de normalizar)
    std::vector<double> saliency(static_cast<size_t>(width * height), 0.0);

    double max_saliency = 0.0;

    // Para cada aresta do grafo original, calcular saliencia via LCA
    for (const Edge& edge : graph.edges) {
        int u = edge.u;
        int v = edge.v;

        // Encontrar o LCA de u e v na hierarquia
        int lca_node = hierarchy.lca(u, v);
        double sal = hierarchy.nodes[lca_node].merge_level;

        // Determinar pixel(es) de fronteira
        // Coordenadas de u e v
        int ux = u % width;
        int uy = u / width;
        int vx = v % width;
        int vy = v / width;

        // Para arestas horizontais (u e v na mesma linha):
        //   fronteira no pixel da direita (maior x)
        // Para arestas verticais (u e v na mesma coluna):
        //   fronteira no pixel de baixo (maior y)
        // Para arestas diagonais:
        //   fronteira em ambos os pixels nao-origem

        // Marcar saliencia no pixel v (ou no pixel com coordenadas maiores)
        // Usamos o maximo caso multiplas arestas contribuam
        int border_x, border_y;

        if (uy == vy) {
            // Aresta horizontal: fronteira no pixel da direita
            border_x = std::max(ux, vx);
            border_y = uy;
            int idx = border_y * width + border_x;
            saliency[idx] = std::max(saliency[idx], sal);
        } else if (ux == vx) {
            // Aresta vertical: fronteira no pixel de baixo
            border_x = ux;
            border_y = std::max(uy, vy);
            int idx = border_y * width + border_x;
            saliency[idx] = std::max(saliency[idx], sal);
        } else {
            // Aresta diagonal: marcar em ambos os pixels nao-origem
            // Pixel v
            int idx_v = vy * width + vx;
            saliency[idx_v] = std::max(saliency[idx_v], sal);
            // Pixel u (tambem contribui para fronteira)
            int idx_u = uy * width + ux;
            saliency[idx_u] = std::max(saliency[idx_u], sal);
        }

        max_saliency = std::max(max_saliency, sal);
    }

    // Normalizar para 0-255
    Image result(width, height, 1);

    if (max_saliency > 0.0) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                uint8_t val = static_cast<uint8_t>(
                    std::round(saliency[idx] / max_saliency * 255.0));
                result.set(x, y, 0, val);
            }
        }
    }

    return result;
}

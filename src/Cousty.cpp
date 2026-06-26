#include "../include/Cousty.hpp"
#include "../include/Graph.hpp"
#include "../include/Hierarchy.hpp"
#include "../include/Kruskal.hpp"
#include "../include/SaliencyMap.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>

using namespace std;

/**
 * @brief Executa o pipeline completo de segmentacao hierarquica de Cousty.
 *
 * 1. Constroi grafo da imagem.
 * 2. Calcula MST com Kruskal.
 * 3. Constroi hierarquia (BPT).
 * 4. Corta hierarquia no nivel lambda.
 * 5. (Opcional) Gera saliency map.
 * 6. Gera imagem colorizada.
 * 7. Mede tempo de execucao.
 *
 * @param image Imagem de entrada.
 * @param params Parametros do pipeline.
 * @return SegmentationResult com labels, segmentos, tempo e imagens.
 */
SegmentationResult cousty_segment(const Image& image,
                                  const CoustyParams& params) {
    auto t_start = chrono::high_resolution_clock::now();

    SegmentationResult result;

    // 1. Construir grafo da imagem
    Connectivity conn = (params.connectivity == 4)
                            ? Connectivity::FOUR
                            : Connectivity::EIGHT;
    ImageGraph graph(image, conn);

    // 2. Calcular MST com Kruskal
    MSTResult mst = kruskal_mst(graph.num_vertices, graph.edges);

    double min_mst_weight = std::numeric_limits<double>::infinity();
    double max_mst_weight = 0.0;
    double sum_mst_weight = 0.0;
    for (const Edge& edge : mst.mst_edges) {
        min_mst_weight = std::min(min_mst_weight, edge.weight);
        max_mst_weight = std::max(max_mst_weight, edge.weight);
        sum_mst_weight += edge.weight;
    }

    std::cout << "[Cousty] MST weights: min=" << std::fixed << std::setprecision(3)
              << min_mst_weight << ", max=" << max_mst_weight
              << ", avg=" << (mst.mst_edges.empty() ? 0.0 : sum_mst_weight / mst.mst_edges.size())
              << std::defaultfloat << std::endl;

    // 3. Construir hierarquia (BPT) a partir da MST
    HierarchicalSegmentation hierarchy;
    hierarchy.build_from_mst(mst.mst_edges, graph.num_vertices);

    // 4. Cortar hierarquia no nivel lambda
    result.labels = hierarchy.cut_at_level(params.lambda);

    // Contar segmentos unicos
    set<int> unique_labels(result.labels.begin(), result.labels.end());
    result.num_segments = static_cast<int>(unique_labels.size());

    // 5. Gerar saliency map (opcional)
    if (params.compute_saliency) {
        result.saliency_image = compute_saliency_map(
            hierarchy, graph, image.width, image.height);
    }

    // 6. Gerar imagem colorizada da segmentacao
    result.segmentation_image = colorize_segmentation(
        result.labels, image.width, image.height);

    // 7. Medir tempo de execucao
    auto t_end = chrono::high_resolution_clock::now();
    result.elapsed_ms = chrono::duration<double, milli>(
                            t_end - t_start)
                            .count();

    cout << "[Cousty] Segmentacao concluida:" << endl;
    cout << "  Dimensoes: " << image.width << " x " << image.height
         << " (" << image.channels << " canais)" << endl;
    cout << "  Lambda: " << params.lambda << endl;
    cout << "  Conectividade: " << params.connectivity << endl;
    cout << "  Segmentos: " << result.num_segments << endl;
    cout << "  Tempo: " << result.elapsed_ms << " ms" << endl;

    return result;
}

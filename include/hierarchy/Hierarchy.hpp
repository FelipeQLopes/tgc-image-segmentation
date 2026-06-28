#ifndef HIERARCHY_HPP
#define HIERARCHY_HPP

#include "graph/Edge.hpp"

#include <vector>

/**
 * @brief No da arvore de particao binaria (BPT / dendrograma).
 *
 * Nos folha representam pixels individuais (ids 0..n-1).
 * Nos internos representam merges de componentes (ids n..2n-2).
 */
struct HierarchyNode {
    int    id;           // identificador do no
    int    left_child;   // id do filho esquerdo  (-1 se folha)
    int    right_child;  // id do filho direito   (-1 se folha)
    int    parent;       // id do no pai          (-1 se raiz)
    double merge_level;  // peso da aresta MST que causou o merge (lambda)
    int    size;         // numero de pixels na regiao

    /** @brief Construtor padrao (folha). */
    HierarchyNode()
        : id(-1), left_child(-1), right_child(-1), parent(-1),
          merge_level(0.0), size(1) {}

    /**
     * @brief Construtor completo.
     * @param id Identificador do no.
     * @param left Id do filho esquerdo.
     * @param right Id do filho direito.
     * @param level Nivel de merge (peso da aresta MST).
     * @param sz Numero de pixels na regiao.
     */
    HierarchyNode(int id, int left, int right, double level, int sz)
        : id(id), left_child(left), right_child(right), parent(-1),
          merge_level(level), size(sz) {}

    /** @brief Retorna true se o no e uma folha (sem filhos). */
    bool is_leaf() const { return left_child == -1 && right_child == -1; }
};

/**
 * @brief Segmentacao hierarquica baseada em Binary Partition Tree.
 *
 * Construida a partir da MST, permite obter segmentacoes em diferentes
 * escalas via corte horizontal no nivel lambda.
 */
class HierarchicalSegmentation {
public:
    std::vector<HierarchyNode> nodes; // nos folha (0..n-1) + internos (n..2n-2)
    int root;                         // id do no raiz

    /** @brief Construtor padrao. */
    HierarchicalSegmentation();

    /**
     * @brief Constroi a hierarquia (BPT) a partir das arestas da MST.
     *
     * Processa as arestas em ordem crescente de peso, criando nos internos
     * que representam o merge de componentes conexas.
     *
     * @param mst_edges Arestas da MST (serao ordenadas internamente).
     * @param num_vertices Numero total de vertices (pixels).
     */
    void build_from_mst(std::vector<Edge> mst_edges, int num_vertices);

    /**
     * @brief Corta a hierarquia no nivel lambda.
     *
     * Percorre a arvore de cima para baixo: se merge_level > lambda,
     * desce para os filhos; senao, todos os pixels abaixo recebem
     * o mesmo label (o id do no de corte).
     *
     * @param lambda Nivel de corte.
     * @return Vetor de labels (um por pixel, tamanho = num folhas).
     */
    std::vector<int> cut_at_level(double lambda) const;

    /**
     * @brief Retorna quantos segmentos o corte em lambda produz.
     * @param lambda Nivel de corte.
     * @return Numero de segmentos.
     */
    int num_segments_at_level(double lambda) const;

    /**
     * @brief Encontra o menor ancestral comum (LCA) de dois nos folha.
     * @param u Id do primeiro no (tipicamente um pixel).
     * @param v Id do segundo no (tipicamente um pixel).
     * @return Id do LCA na hierarquia.
     */
    int lca(int u, int v) const;

private:
    int num_leaves_; // numero de nos folha (pixels)
    std::vector<Edge> mst_edges_; // arestas da MST usadas para o corte por limiar

    // Tabelas para Binary Lifting LCA
    std::vector<int> depth;
    std::vector<std::vector<int>> up;
};

#endif // HIERARCHY_HPP

#include "hierarchy/Hierarchy.hpp"
#include "graph/DisjointSet.hpp"

#include <algorithm>
#include <vector>


// =========================================================================
// HierarchicalSegmentation
// =========================================================================

HierarchicalSegmentation::HierarchicalSegmentation()
    : root(-1), num_leaves_(0) {}

/**
 * @brief Constroi a BPT a partir das arestas da MST.
 *
 * 1. Cria n nos folha (um por pixel), ids de 0 a n-1.
 * 2. Ordena arestas da MST por peso crescente.
 * 3. Para cada aresta (u, v, w):
 *    - Encontra representantes Cu, Cv via DisjointSet.
 *    - Cria no interno com filhos = nos das componentes Cu e Cv.
 *    - Une as componentes e atualiza o mapeamento.
 * 4. root = ultimo no criado.
 *
 * @param mst_edges Arestas da MST.
 * @param num_vertices Numero total de vertices (pixels).
 */
void HierarchicalSegmentation::build_from_mst(std::vector<Edge> mst_edges,
                                               int num_vertices) {
    num_leaves_ = num_vertices;
    int n = num_vertices;
    mst_edges_ = std::move(mst_edges);
    std::sort(mst_edges_.begin(), mst_edges_.end());

    // A BPT tera n folhas + (n-1) nos internos = 2n-1 nos no total
    nodes.clear();
    nodes.reserve(static_cast<size_t>(2 * n - 1));

    // Passo 1: Criar n nos folha
    for (int i = 0; i < n; ++i) {
        HierarchyNode leaf;
        leaf.id = i;
        leaf.left_child = -1;
        leaf.right_child = -1;
        leaf.parent = -1;
        leaf.merge_level = 0.0;
        leaf.size = 1;
        nodes.push_back(leaf);
    }

    // Passo 2: Inicializar DisjointSet
    DisjointSet dsu(n);

    // Passo 4: Mapeamento representante -> id do no na hierarquia
    // node_of_component[representante] = id do no que representa essa componente
    std::vector<int> node_of_component(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        node_of_component[i] = i; // inicialmente cada pixel e seu proprio no
    }

    int next_id = n;

    // Passo 5: Processar cada aresta da MST
    for (const Edge& edge : mst_edges_) {
        int cu = dsu.find(edge.u);
        int cv = dsu.find(edge.v);

        if (cu == cv) {
            continue; // ja estao na mesma componente (nao deveria ocorrer na MST)
        }

        // Criar novo no interno
        int left_id = node_of_component[cu];
        int right_id = node_of_component[cv];
        int new_size = dsu.component_size(cu) + dsu.component_size(cv);

        HierarchyNode internal(next_id, left_id, right_id,
                               edge.weight, new_size);
        nodes.push_back(internal);

        // Definir parent dos filhos
        nodes[left_id].parent = next_id;
        nodes[right_id].parent = next_id;

        // Unir componentes
        dsu.unite(cu, cv);

        // Atualizar mapeamento: o novo representante aponta para o novo no
        int new_rep = dsu.find(edge.u);
        node_of_component[new_rep] = next_id;

        ++next_id;
    }

    // Passo 6: A raiz e o ultimo no criado
    root = next_id - 1;

    // Passo 7: Pre-computar Binary Lifting para LCA O(1) / O(log V)
    int total_nodes = next_id;
    int LOG = 21; // 2^21 > 2 milhoes, suficiente para imagens grandes
    
    depth.assign(total_nodes, 0);
    up.assign(total_nodes, std::vector<int>(LOG, root));

    if (root >= 0) {
        depth[root] = 0;
        for (int j = 0; j < LOG; ++j) {
            up[root][j] = root;
        }

        // Os nos foram criados em ordem de merge (filhos < pais).
        // Entao podemos calcular a profundidade descendo do root ate 0.
        for (int i = root - 1; i >= 0; --i) {
            int p = nodes[i].parent;
            if (p == -1) p = i; // fallback se for floresta
            
            depth[i] = depth[p] + 1;
            up[i][0] = p;
            for (int j = 1; j < LOG; ++j) {
                up[i][j] = up[up[i][j - 1]][j - 1];
            }
        }
    }
}

/**
 * @brief Corta a hierarquia no nivel lambda.
 *
 * Usa abordagem iterativa top-down para evitar stack overflow em
 * imagens grandes. Como o id do no pai e sempre maior que o id
 * dos filhos, podemos iterar em ordem decrescente.
 *
 * @param lambda Nivel de corte.
 * @return Vetor de labels (tamanho = num_leaves_).
 */
std::vector<int> HierarchicalSegmentation::cut_at_level(double lambda) const {
    std::vector<int> labels(static_cast<size_t>(num_leaves_), -1);

    if (num_leaves_ <= 0) {
        return labels;
    }

    DisjointSet dsu(num_leaves_);
    for (const Edge& edge : mst_edges_) {
        if (edge.weight <= lambda) {
            dsu.unite(edge.u, edge.v);
        }
    }

    for (int i = 0; i < num_leaves_; ++i) {
        labels[i] = dsu.find(i);
    }

    return labels;
}

/**
 * @brief Retorna quantos segmentos o corte em lambda produz.
 *
 * Em uma BPT baseada em MST, cada no interno com merge_level <= lambda
 * corresponde a uma aresta da MST aceita. Como comecamos com n segmentos,
 * cada aresta aceita reduz o numero de segmentos em 1.
 * Portanto, segmentos = n - (merges aceitos).
 *
 * @param lambda Nivel de corte.
 * @return Numero de segmentos.
 */
int HierarchicalSegmentation::num_segments_at_level(double lambda) const {
    if (num_leaves_ <= 0) {
        return 0;
    }

    DisjointSet dsu(num_leaves_);
    for (const Edge& edge : mst_edges_) {
        if (edge.weight <= lambda) {
            dsu.unite(edge.u, edge.v);
        }
    }

    return dsu.num_components();
}

/**
 * @brief Encontra o menor ancestral comum (LCA) de dois nos.
 *
 * Utiliza o algoritmo de Binary Lifting (pre-computado na construcao),
 * permitindo encontrar o LCA em O(log V). Ideal para a geracao do
 * Saliency Map, que faz milhoes de consultas de LCA.
 *
 * @param u Id do primeiro no.
 * @param v Id do segundo no.
 * @return Id do LCA.
 */
int HierarchicalSegmentation::lca(int u, int v) const {
    if (u == v) return u;
    
    if (depth[u] < depth[v]) {
        std::swap(u, v);
    }
    
    int LOG = 21;
    
    // 1. Igualar as profundidades subindo u
    for (int j = LOG - 1; j >= 0; --j) {
        if (depth[u] - (1 << j) >= depth[v]) {
            u = up[u][j];
        }
    }
    
    if (u == v) return u;
    
    // 2. Subir ambos simultaneamente ate estarem logo abaixo do LCA
    for (int j = LOG - 1; j >= 0; --j) {
        if (up[u][j] != up[v][j]) {
            u = up[u][j];
            v = up[v][j];
        }
    }
    
    return up[u][0];
}


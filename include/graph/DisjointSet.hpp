#ifndef DISJOINT_SET_HPP
#define DISJOINT_SET_HPP

#include <vector>

/**
 * @brief Estrutura de conjuntos disjuntos (Union-Find / Disjoint Set Union).
 * Suporta as otimizações de Path Compression e Union by Rank.
 */
class DisjointSet {
public:
    /**
     * @brief Construtor que inicializa a estrutura com 'n' elementos.
     * Cada elemento começa em seu próprio conjunto de tamanho 1.
     * @param n Número total de elementos.
     */
    explicit DisjointSet(int n);

    /**
     * @brief Encontra o representante (raiz) do conjunto que contém 'i'.
     * Aplica compressão de caminho (path compression) recursivamente.
     * @param i Elemento a ser buscado.
     * @return Id do representante do conjunto.
     */
    int find(int i);

    /**
     * @brief Une os conjuntos que contêm os elementos 'i' e 'j'.
     * Aplica união por rank (union by rank) para manter a árvore balanceada.
     * @param i Primeiro elemento.
     * @param j Segundo elemento.
     * @return true se ocorreu união real (estavam em componentes diferentes),
     *         false caso contrário.
     */
    bool unite(int i, int j);

    /**
     * @brief Retorna o tamanho do componente contendo 'i'.
     * @param i Elemento cuja componente será consultada.
     * @return Quantidade de elementos na componente.
     */
    int component_size(int i);

    /**
     * @brief Retorna o número total de componentes independentes ativos.
     * @return Quantidade de componentes.
     */
    int num_components() const;

private:
    std::vector<int> parent; // parent[i] guarda o pai de i na floresta
    std::vector<int> rank;   // rank[i] guarda o limite superior para a altura da subárvore de raiz i
    std::vector<int> size;   // size[i] guarda o tamanho da componente se i for o representante (raiz)
    int count;               // Quantidade de componentes disjuntos ativos
};

#endif // DISJOINT_SET_HPP

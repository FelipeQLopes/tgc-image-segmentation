# PR: infra: estrutura de dados Union-Find (Disjoint Set Union) (#4)

## Descrição
Este PR implementa a estrutura de dados **Union-Find** (`DisjointSet`), que gerencia conjuntos disjuntos de elementos. Ela é a fundação para a construção da Árvore Geradora Mínima (MST) via algoritmo de Kruskal (compartilhado entre o Artigo A e B) e para o algoritmo de Felzenszwalb-Huttenlocher (Artigo A).

A estrutura foi projetada visando eficiência máxima, utilizando as otimizações clássicas de **Path Compression** e **Union by Rank**, resultando em complexidade quase constante $O(\alpha(N))$ por operação.

---

## Decisões de Design

### 1. União por Rank (Union by Rank)
**Por quê:** Para evitar que a floresta de conjuntos disjuntos degenere em árvores com caminhos longos (que aumentariam a busca para $O(N)$), associamos a cada elemento um "rank" (estimativa da altura da subárvore). Ao unir dois conjuntos, a raiz do conjunto de menor rank é apontada para a de maior rank, mantendo as árvores balanceadas.

### 2. Compressão de Caminho (Path Compression)
**Por quê:** Na operação de busca (`find(i)`), apontamos todos os nós percorridos diretamente para a raiz do conjunto. Isso achata a árvore dinamicamente, de forma que buscas futuras tomem tempo $O(1)$.

### 3. Rastreamento dinâmico do tamanho de componentes (`size`)
**Por quê:** O algoritmo de Felzenszwalb-Huttenlocher exige saber o tamanho de uma componente (em pixels) a cada passo para calcular seu limiar interno. Ao invés de recalcular isso percorrendo os nós, a estrutura mantém um vetor `size` atualizado na raiz da componente no momento do `unite`.

### 4. Rastreamento do número total de componentes (`count`)
**Por quê:** O algoritmo de Cousty (QFZ) e o de Kruskal precisam controlar o progresso do merge de maneira global. O contador `count` é inicializado com `n` e decrementado a cada `unite` bem-sucedido, permitindo saber o número de componentes ativas em $O(1)$.

---

## Detalhes de Implementação

### Estrutura da Classe (`include/DisjointSet.hpp` + `src/DisjointSet.cpp`)

```
DisjointSet
├── Construtor(n) → inicializa parent[i]=i, rank[i]=0, size[i]=1, count=n
├── find(i) → retorna a raiz/líder com compressão de caminho recursiva
├── unite(i, j) → une conjuntos por rank e decrementa contador global
├── component_size(i) → retorna o tamanho do grupo contendo 'i'
└── num_components() → retorna a quantidade de grupos independentes ativos
```

---

## Testes Automatizados

Foi criado o arquivo `tests/test_disjoint_set.cpp` para validar as propriedades fundamentais da estrutura de dados:

| # | Teste | O que valida |
|---|-------|-------------|
| 1 | Inicialização | Todos os nós são líderes de si mesmos, tamanho individual = 1, contagem = N |
| 2 | União básica | `unite` retorna true em merges válidos e false em redundantes; líderes se igualam |
| 3 | Tamanho de componentes | Tamanho aumenta corretamente com merges e reflete a soma de subcomponentes |
| 4 | União por rank | A união de subárvores preserva o balanceamento e o representante correto |

### Como executar

```bash
# Compilar e rodar os testes específicos do DisjointSet
make test_ds

# Compilar e executar toda a suíte de testes (Image, Graph e DisjointSet)
make test
```

---

## Critérios de Aceite — Validação

| Critério | Status | Evidência |
|----------|--------|-----------|
| Path compression implementado | ✅ | Validado nos testes unitários e na recursão com atribuição em `find(parent[i])` |
| Union by rank implementado | ✅ | Validado no teste de União por Rank |
| Tamanho de cada componente rastreado | ✅ | Validado no teste de Tamanho de componentes |
| Número total de componentes atualizado | ✅ | Rastreamento testado e validado em `test_basic_union` |
| Sem regressões no projeto | ✅ | `make test` executa com sucesso todos os testes antigos sem qualquer falha |

---

## Dependências

- Nenhuma dependência externa adicional. Usa apenas a biblioteca padrão C++ (`<vector>`).

---

## Issues

Closes #4


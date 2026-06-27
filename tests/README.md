# Guia de Testes e Estado da Implementação

Este diretório contém a suíte de testes unitários do projeto de **Segmentação de Imagens Baseada em Grafos** (Algoritmo de Cousty). Este documento serve como um guia técnico para entender o estado atual do desenvolvimento, a estrutura de cada módulo implementado e o detalhamento dos testes que garantem a corretude física e matemática dos algoritmos.

---

## Estado Atual do Projeto (Módulos Implementados)

O pipeline completo de segmentação hierárquica foi implementado. O projeto está estruturado nos seguintes componentes:

```
tgc-image-segmentation/
├── include/
│   ├── image.hpp           # Representação e I/O de Imagens (stb)
│   ├── Edge.hpp            # Estrutura de Aresta Ponderada
│   ├── Graph.hpp           # Conversão de Imagem para Grafo
│   ├── DisjointSet.hpp     # DSU (Union-Find) com Otimizações
│   ├── Kruskal.hpp         # Algoritmo de Kruskal (MST)
│   ├── PriorityQueue.hpp   # Fila de Prioridade Mínima (lazy)
│   ├── Hierarchy.hpp       # Árvore Binária de Partições (BPT)
│   ├── SaliencyMap.hpp     # Mapa de Saliência baseado na BPT
│   └── Cousty.hpp          # Pipeline completo de segmentação
├── src/
│   ├── Image.cpp           # Implementação de I/O e manipulação
│   ├── Graph.cpp           # Implementação do Grafo de Imagem
│   ├── DisjointSet.cpp     # Implementação das operações do DSU
│   ├── Kruskal.cpp         # Implementação da MST por Kruskal
│   ├── PriorityQueue.cpp   # Implementação da fila de prioridade
│   ├── Hierarchy.cpp       # Implementação da BPT e cortes
│   ├── SaliencyMap.cpp     # Implementação do mapa de saliência
│   ├── Cousty.cpp          # Orquestração do pipeline completo
│   └── main.cpp            # Ponto de entrada da aplicação
└── tests/
    ├── test_image.cpp           # Testes unitários do módulo Image
    ├── test_graph.cpp           # Testes unitários do módulo Graph
    ├── test_disjoint_set.cpp    # Testes unitários do módulo DSU
    ├── test_kruskal.cpp         # Testes unitários do módulo Kruskal
    ├── teste_priority_queue.cpp # Testes unitários da PriorityQueue
    ├── test_hierarchy.cpp       # Testes unitários da Hierarchy (BPT)
    └── test_cousty.cpp          # Testes de integração do pipeline Cousty
```

---

## Arquitetura dos Testes

Os testes são escritos em C++ puro (C++17) utilizando a biblioteca padrão `<cassert>`. Cada arquivo de teste possui uma estrutura autocontida que:
1. Define funções para testes específicos.
2. Usa macros utilitárias (`TEST` e `PASS`) para registrar a execução e o sucesso das validações no terminal.
3. Retorna código de erro `1` em caso de falha e `0` em caso de sucesso geral, integrando-se nativamente com ferramentas de automação (como Make e sistemas de CI/CD).

---

## Detalhamento das Validações por Módulo

### 1. Módulo Image (`tests/test_image.cpp`)
Valida as operações de leitura, escrita, conversão de espaço de cores e colorização da segmentação.

*   **Acessores `at()` e `set()`:** Garante que a matriz de pixels é lida e escrita corretamente na disposição *Row-Major* com suporte a múltiplos canais, validando também o controle de limites (*bounds-checking*).
*   **Salvamento e Carregamento de PNG (Grayscale e RGB):** Faz um teste de *round-trip* (salva uma matriz gerada artificialmente em disco, carrega de volta e compara bit a bit) para garantir que não há perda de dados no I/O.
*   **Tratamento de Exceções:** Verifica se o carregador lança um erro `std::runtime_error` apropriado ao tentar ler arquivos inexistentes.
*   **Conversão para Tons de Cinza:** Valida a conversão usando os coeficientes de luminância do padrão **ITU-R BT.601** ($Y = 0.299R + 0.587G + 0.114B$), que reflete a percepção do olho humano.
*   **Colorização de Segmentação:** Valida se labels distintos recebem cores aleatórias consistentes (utilizando uma semente de números pseudo-aleatórios fixa para reprodutibilidade) e se gera erro caso os tamanhos de matriz sejam incompatíveis.

---

### 2. Módulo Graph (`tests/test_graph.cpp`)
Valida o mapeamento bidirecional da imagem como um grafo ponderado não-direcionado.

*   **Conversão de Coordenadas:** Garante a corretude das funções de mapeamento indexado:
    $$\text{id} = y \times \text{width} + x$$
    e suas respectivas operações inversas de decodificação.
*   **Contagem de Arestas (Vizinhança 4 e 8):** Valida a física de contagem para imagens de dimensões conhecidas. Para uma imagem $W \times H$:
    *   **Vizinhança-4:** Espera-se $W(H-1) + H(W-1)$ arestas.
    *   **Vizinhança-8:** Espera-se $W(H-1) + H(W-1) + 2(W-1)(H-1)$ arestas.
*   **Prevenção de Duplicidade:** Garante que a inserção de arestas de vizinhança é *forward-only* (direita, baixo, diagonais inferiores), adicionando cada aresta física exatamente uma única vez na lista de arestas.
*   **Cálculo dos Pesos:** Valida se a diferença absoluta de intensidade é usada para imagens de 1 canal, e a distância euclidiana no espaço tridimensional RGB para imagens de 3 canais.
*   **Ordenação de Arestas:** Garante que o método `sort_edges` ordena a lista por ordem estritamente crescente de peso (requisito primordial para o algoritmo de Kruskal).
*   **Lista de Adjacência:** Verifica se a representação em lista de adjacência é populada de maneira bidirecional e simétrica.

---

### 3. Módulo Disjoint Set Union (`tests/test_disjoint_set.cpp`)
Valida a lógica matemática de fusão e busca de componentes conexas de maneira eficiente.

*   **Inicialização:** Verifica se ao construir um conjunto de tamanho $N$, cada elemento é inicialmente seu próprio representante raiz com tamanho unitário de componente ($size = 1$) e o contador de componentes é igual a $N$.
*   **Operações de União e Busca:** Valida se a chamada a `unite(u, v)` funde corretamente os conjuntos e faz com que `find(u)` e `find(v)` retornem a mesma raiz. Também testa o retorno de status da operação (se houve união real ou se os nós já pertenciam ao mesmo grupo).
*   **Rastreamento de Tamanho:** Garante que o método `component_size` retorna o somatório exato de nós nas componentes fundidas após sucessivos merges de árvores.
*   **Contador Global:** Garante que o total de componentes independentes (`num_components`) é decrementado apropriadamente apenas quando merges reais ocorrem.
*   **Union by Rank:** Garante que a árvore menor é sempre conectada sob a raiz da árvore maior, prevenindo degradação da altura da árvore.

---

### 4. Módulo Kruskal (`tests/test_kruskal.cpp`)
Valida a construção da Árvore Geradora Mínima (MST) pelo algoritmo de Kruskal.

*   **Grafo Linear sem Ciclos:** Garante que todas as arestas de um grafo sem ciclos são incluídas na MST e que o peso total é calculado corretamente.
*   **Grafo Cíclico Triangular:** Valida que a aresta de maior peso é corretamente rejeitada em um grafo com ciclo, resultando em exatamente $N-1$ arestas na MST.
*   **Grade 3×3 Simulando Imagem:** Testa um grafo de 9 vértices com arestas horizontais e verticais (similar a uma imagem real), verificando que a MST contém exatamente 8 arestas e o peso total calculado manualmente é reproduzido.

---

### 5. Módulo PriorityQueue (`tests/teste_priority_queue.cpp`)
Valida a fila de prioridade mínima com estratégia *lazy deletion*.

*   **Fila vazia na criação:** Verifica que uma fila recém-criada reporta `empty() == true`.
*   **`decrease_key` e ordem de extração:** Insere 4 nós com custos distintos via `decrease_key` e valida que `pop()` os retorna em ordem crescente de custo.
*   **Lazy `decrease_key` ignora entradas obsoletas:** Após diminuir a chave de um nó duas vezes, confirma que apenas a entrada mais recente (menor custo) é retornada pelo `pop()`, e que entradas obsoletas são descartadas silenciosamente.
*   **Exceção em nó inválido:** Garante que `decrease_key` lança `std::out_of_range` ao receber um índice de nó fora dos limites da fila.

---

### 6. Módulo Hierarchy / BPT (`tests/test_hierarchy.cpp`)
Valida a construção da Árvore Binária de Partições (BPT) a partir da MST e as operações de corte hierárquico.

*   **Estrutura da BPT — Grafo Linear de 3 Vértices:** Verifica a estrutura completa do dendrograma: número total de nós ($2N-1$), identificação correta da raiz, distinção entre nós folha e internos, e valores exatos de `merge_level` e `size` em cada nó interno.
*   **`cut_at_level(0)` — Máxima Granularidade:** Garante que um corte com $\lambda = 0$ retorna cada pixel como um segmento individual, produzindo $N$ labels únicos.
*   **`cut_at_level(+\infty)` — Mínima Granularidade:** Garante que um corte com $\lambda$ muito alto funde todos os pixels em um único segmento.
*   **Monotonia dos Cortes:** Para uma sequência de $\lambda$s crescentes, valida que o número de segmentos é estritamente não-crescente. Verifica valores exatos de `num_segments_at_level` nos limiares de merge.
*   **Grade 2×2 — Hierarquia Completa:** Testa a hierarquia completa de uma grade 2×2 com pesos conhecidos, verificando os nós internos e validando os cortes em $\lambda = 0$ (3 segmentos) e $\lambda = 5$ (1 segmento).
*   **LCA — Menor Ancestral Comum:** Valida o método `lca(u, v)` para todos os pares de folhas em um grafo linear, incluindo o caso `lca(u, u) == u`.
*   **Contagem de Nós Internos para $n$ Folhas:** Para $n \in [2, 10]$, verifica que a BPT tem exatamente $2n-1$ nós totais e $n-1$ nós internos.

---

### 7. Pipeline Cousty (`tests/test_cousty.cpp`)
Valida o pipeline de segmentação ponta a ponta integrando todos os módulos anteriores.

*   **Pipeline com Imagem Cinza 4×4:** Executa o pipeline completo (grafo → MST → BPT → corte) em uma imagem sintética com gradiente. Verifica o número de labels, a existência de pelo menos 1 segmento, o tempo de execução positivo, e as dimensões corretas das imagens de saída (segmentação RGB e mapa de saliência grayscale).
*   **Pipeline com Imagem RGB 3×3 Bicolor:** Executa o pipeline com conectividade 8 em uma imagem com duas regiões de cores distintas (vermelho e azul), validando a integridade básica do resultado.
*   **Monotonia do Parâmetro $\lambda$:** Com a mesma imagem, executa o pipeline com $\lambda = 0$, $\lambda = 30$ e $\lambda = 10^6$, verificando que o número de segmentos é monótono não-crescente. Valida os casos extremos: $\lambda = 0 \Rightarrow 16$ segmentos (um por pixel) e $\lambda = 10^6 \Rightarrow 1$ segmento.
*   **Mapa de Saliência Opcional:** Verifica que, ao desativar `compute_saliency`, a imagem de saliência retornada tem dimensões nulas (largura e altura iguais a 0), enquanto a segmentação continua funcionando normalmente.
*   **Conectividade 4 vs. 8:** Executa o pipeline nas duas modalidades de vizinhança com o mesmo $\lambda$ e verifica que ambos produzem resultados válidos (dimensões corretas e pelo menos 1 segmento).
*   **Imagem Uniforme:** Em uma imagem 3×3 com todos os pixels de mesmo valor (pesos de arestas todos iguais a 0), verifica que com $\lambda = 0$ todos os pixels são fundidos em um único segmento.

---

## Como Compilar e Rodar os Testes

Todos os testes são compilados através do compilador padrão `g++` suportando C++17. A execução é facilitada via comandos do `Makefile`.

### Rodar a suíte completa
Para compilar e executar todos os testes sequencialmente:
```bash
make test
```

### Rodar testes específicos de um módulo
Caso queira compilar e rodar apenas o contexto de validação de um módulo em desenvolvimento:

```bash
# Módulo Image
make test_image

# Módulo Graph
make test_graph

# Módulo DisjointSet
make test_ds

# Módulo Kruskal
make test_kruskal

# Módulo PriorityQueue
make test_pq

# Módulo Hierarchy (BPT)
make test_hierarchy

# Pipeline Cousty (integração)
make test_cousty
```

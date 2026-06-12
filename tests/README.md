# Guia de Testes e Estado da Implementação

Este diretório contém a suíte de testes unitários do projeto de **Segmentação de Imagens Baseada em Grafos**. Este documento serve como um guia técnico para entender o estado atual do desenvolvimento, a estrutura de cada módulo implementado e o detalhamento dos testes que garantem a corretude física e matemática dos algoritmos.

---

## Estado Atual do Projeto (Módulos Implementados)

Até o momento, a infraestrutura básica e a base matemática para os algoritmos de segmentação foram concluídas. O projeto está estruturado nos seguintes componentes:

```
tgc-image-segmentation/
├── include/
│   ├── image.hpp       # Representação e I/O de Imagens (stb)
│   ├── Edge.hpp        # Estrutura de Aresta Ponderada
│   ├── Graph.hpp       # Conversão de Imagem para Grafo
│   └── DisjointSet.hpp # DSU (Union-Find) com Otimizações
├── src/
│   ├── Image.cpp       # Implementação de I/O e manipulação
│   ├── Graph.cpp       # Implementação do Grafo de Imagem
│   └── DisjointSet.cpp # Implementação das operações do DSU
└── tests/
    ├── test_image.cpp  # Testes unitários do módulo Image
    ├── test_graph.cpp  # Testes unitários do módulo Graph
    └── test_disjoint_set.cpp # Testes unitários do módulo DSU
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
# Apenas Módulo Image
make test_image

# Apenas Módulo Graph
make test_graph

# Apenas Módulo DisjointSet
make test_ds
```

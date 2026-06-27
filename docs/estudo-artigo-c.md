# Image Foresting Transform (IFT)

## 1. Introdução

O **Image Foresting Transform (IFT)** é um algoritmo de segmentação de imagens proposto por Falcão, Stolfi e Lotufo (2004). Seu principal objetivo é dividir uma imagem em regiões por meio de uma modelagem baseada em grafos e da propagação de caminhos ótimos a partir de um conjunto de sementes (*seeds*).

O IFT pode ser visto como uma **generalização do algoritmo de Dijkstra**, pois, ao invés de calcular caminhos mínimos a partir de uma única origem, permite a propagação simultânea de múltiplas sementes utilizando diferentes funções de custo de caminho. O resultado é uma **Floresta de Caminhos Ótimos (Optimum Path Forest - OPF)**, em que cada árvore representa uma região da segmentação.

---

# 2. Modelagem da imagem como grafo

No IFT, uma imagem é representada como um grafo ponderado (G=(V,E)).

* Cada pixel corresponde a um vértice.
* Pixels vizinhos são conectados por arestas.
* O peso de cada aresta representa a diferença entre dois pixels vizinhos.

A vizinhança pode ser:

* 4-conectada (cima, baixo, esquerda e direita);
* 8-conectada (incluindo as diagonais).

Para imagens em níveis de cinza, o peso normalmente é calculado por:

[
w(s,t)=|I(s)-I(t)|
]

onde (I(s)) representa a intensidade do pixel.

Para imagens coloridas, utiliza-se a distância Euclidiana entre os vetores RGB:

[
w(s,t)=\sqrt{(R_s-R_t)^2+(G_s-G_t)^2+(B_s-B_t)^2}
]

Essa representação é independente da dimensão da imagem e permite aplicar o algoritmo tanto em imagens monocromáticas quanto coloridas.

---

# 3. Conceito central do IFT

O algoritmo inicia a partir de um conjunto de sementes.

Cada semente representa uma região inicial da imagem e possui:

* custo igual a zero;
* um rótulo único;
* nenhum predecessor.

Todos os demais pixels começam com custo infinito.

Durante a execução, os custos são propagados pelas arestas do grafo de forma semelhante ao algoritmo de Dijkstra. Cada pixel passa a pertencer à semente que consegue alcançá-lo por um caminho de menor custo segundo a função de custo escolhida.

Ao final do processo, obtém-se uma floresta de caminhos ótimos, onde:

* cada árvore possui exatamente uma semente como raiz;
* todos os pixels pertencem a exatamente uma árvore;
* cada árvore corresponde a uma região segmentada.

---

# 4. Relação com o algoritmo de Dijkstra

O IFT pode ser entendido como uma extensão do algoritmo de Dijkstra.

No algoritmo clássico:

* existe apenas uma origem;
* o custo do caminho é normalmente a soma dos pesos;
* o resultado é uma única árvore de caminhos mínimos.

No IFT:

* existem várias sementes simultaneamente;
* diferentes funções de custo podem ser utilizadas;
* o resultado é uma floresta de caminhos ótimos.

Essa generalização torna o algoritmo adequado para diversos problemas de processamento de imagens, principalmente segmentação por watershed.

---

# 5. Funções de custo

A principal diferença entre diferentes aplicações do IFT está na função utilizada para avaliar o custo de um caminho.

## 5.1 Função f_sum

A função **f_sum** corresponde ao algoritmo de Dijkstra tradicional.

O custo de um caminho é dado pela soma dos pesos das arestas percorridas.

[
f_{sum}(\pi)=\sum_{e\in\pi}w(e)
]

Essa função favorece caminhos cuja distância acumulada seja mínima.

É utilizada em aplicações clássicas de caminhos mínimos.

---

## 5.2 Função f_max

Neste trabalho será utilizada a função **f_max**, empregada na segmentação do tipo Watershed.

Inicialização:

[
f_{max}(\langle t\rangle)=
\begin{cases}
0,& t \text{ é uma semente}\
+\infty,& caso contrário
\end{cases}
]

Durante a propagação:

[
f_{max}(\pi\cdot\langle s,t\rangle)=
\max(f_{max}(\pi),w(s,t))
]

Ao contrário da soma acumulada, o custo de um caminho passa a ser o maior peso encontrado ao longo dele.

Dessa forma, o algoritmo procura caminhos cujo maior obstáculo seja o menor possível.

Essa propriedade faz com que o IFT produza regiões separadas por bordas de alto contraste, sendo amplamente utilizado em algoritmos Watershed.

---

## 5.3 Diferença entre f_sum e f_max

| f_sum                            | f_max                                 |
| -------------------------------- | ------------------------------------- |
| Soma dos pesos das arestas       | Maior peso encontrado no caminho      |
| Equivalente ao Dijkstra clássico | Utilizada em Watershed                |
| Minimiza distância acumulada     | Minimiza o maior obstáculo do caminho |

---

# 6. Seleção de sementes

A qualidade da segmentação depende diretamente da escolha das sementes.

Neste trabalho será utilizada seleção automática baseada nos mínimos regionais do mapa de gradiente.

O procedimento consiste em:

1. calcular o gradiente da imagem;
2. localizar todos os mínimos regionais;
3. transformar cada mínimo regional em uma semente;
4. atribuir um rótulo diferente para cada semente.

Cada semente inicia uma árvore diferente da floresta.

Como consequência, o número de sementes determina diretamente o número de regiões obtidas pela segmentação.

---

# 7. Estruturas produzidas pelo algoritmo

O IFT produz três estruturas principais.

## Mapa de custos (C)

O vetor (C[t]) armazena o menor custo conhecido para alcançar o pixel (t).

Após o término do algoritmo, esse valor corresponde ao custo ótimo do caminho entre a semente e o pixel.

---

## Mapa de predecessores (P)

O vetor (P[t]) guarda o predecessor imediato do pixel (t) no caminho ótimo.

Esses predecessores definem a estrutura da floresta de caminhos ótimos.

---

## Mapa de rótulos (L)

O vetor (L[t]) armazena o rótulo da semente que conquistou o pixel.

Na prática, esse vetor representa a própria segmentação da imagem.

Todos os pixels pertencentes à mesma árvore possuem exatamente o mesmo rótulo.

---

# 8. Pseudocódigo do IFT utilizando f_max

```text
Entrada:
    Grafo G(V,E)
    Conjunto de sementes S
    Peso das arestas w

Inicialização

para cada vértice v

    C[v] <- infinito
    P[v] <- NIL
    L[v] <- indefinido

para cada semente s

    C[s] <- 0
    P[s] <- NIL
    L[s] <- rótulo da semente
    inserir s na fila de prioridade

enquanto a fila não estiver vazia

    remover o vértice p de menor custo

    para cada vizinho q de p

        novo <- max(C[p], w(p,q))

        se novo < C[q]

            C[q] <- novo
            P[q] <- p
            L[q] <- L[p]

            atualizar q na fila

Saída

C : mapa de custos

P : predecessores

L : mapa de rótulos
```

---

# 9. Exemplo de execução

Considere uma imagem 4×4 com duas sementes.

```text
S . . .

. . . .

. . . .

. . . S
```

Inicialmente:

* ambas as sementes possuem custo zero;
* todos os demais pixels possuem custo infinito.

Na primeira iteração, apenas os vizinhos imediatos das sementes recebem novos custos.

```text
S 1 . .

1 . . .

. . . 1

. . 1 S
```

Nas iterações seguintes, os custos continuam sendo propagados pela imagem.

Quando dois caminhos competem pelo mesmo pixel, permanece aquele cujo valor de `f_max` é menor.

Ao final do algoritmo, todos os pixels pertencem à árvore de alguma semente, formando a Floresta de Caminhos Ótimos.

---

# 10. Complexidade

A complexidade do IFT depende da estrutura utilizada para implementar a fila de prioridade.

Quando os pesos das arestas são inteiros e utiliza-se uma **Bucket Queue**, a complexidade é linear:

[
O(n)
]

onde (n) representa o número de pixels da imagem.

Caso seja utilizada uma fila baseada em heap binário, a complexidade passa a ser aproximadamente:

[
O((V+E)\log V)
]

Como imagens possuem grau constante (4 ou 8 vizinhos por pixel), essa complexidade costuma ser representada por (O(n\log n)).

A utilização de Bucket Queue é recomendada pelo artigo original por oferecer melhor desempenho em aplicações de processamento de imagens.

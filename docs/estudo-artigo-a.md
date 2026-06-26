# Segmentação de Imagens Baseada em Grafo
### Documentação do Algoritmo de Felzenszwalb & Huttenlocher (IJCV, 2004)

---

## 1. Modelagem do Grafo G = (V, E)

### 1.1 Transformação da Imagem em Grafo

Uma imagem de dimensões `H × W` pixels é transformada em um grafo não-direcionado
`G = (V, E)` da seguinte forma:

**Pré-processamento:** antes da construção do grafo, a imagem é suavizada com um
filtro Gaussiano de desvio padrão `σ` para reduzir ruídos de alta frequência que
poderiam gerar arestas espúrias entre pixels adjacentes.

```
Imagem original  →  Filtro Gaussiano (σ)  →  Construção de G = (V, E)
```

### 1.2 O que os Vértices Representam

Cada pixel `p = (x, y)` da imagem corresponde a **exatamente um vértice** `v ∈ V`.
Para uma imagem `H × W`, o conjunto de vértices tem cardinalidade `|V| = n = H × W`.

```
Pixel (x, y)  ──→  vértice v_(x,y) ∈ V
```

### 1.3 O que as Arestas Representam

As arestas em `E` conectam pares de **pixels vizinhos** na grade da imagem.
A vizinhança utilizada é a conectividade de 8 (8-neighborhood), onde cada pixel
se conecta com os 8 pixels ao seu redor:

```
┌───┬───┬───┐
│ ↖ │ ↑ │ ↗ │
├───┼───┼───┤
│ ← │ p │ → │   p = pixel central
├───┼───┼───┤
│ ↙ │ ↓ │ ↘ │
└───┴───┴───┘
```

Cada aresta `(v_i, v_j) ∈ E` representa uma **fronteira potencial** entre dois
pixels adjacentes. O número total de arestas é `|E| = m ≈ 4n` (para vizinhança
de 4) ou `m ≈ 8n` (para vizinhança de 8), ou seja, linear em `n`.

> **Nota:** O algoritmo também suporta grafos de vizinhos mais próximos no espaço
> de features `(x, y, r, g, b)`, que capturam propriedades globais da cena.

### 1.4 Cálculo dos Pesos das Arestas

O peso `w(v_i, v_j)` de cada aresta mede a **dissimilaridade** entre os pixels
conectados. Para imagens em **escala de cinza**:

```
w(v_i, v_j) = |I(p_i) - I(p_j)|
```

onde `I(p)` é a intensidade do pixel `p` após a suavização Gaussiana.

Para imagens **coloridas (RGB)**:

```
w(v_i, v_j) = sqrt( (R_i - R_j)² + (G_i - G_j)² + (B_i - B_j)² )
```

Propriedades do peso:
- **w ≥ 0** para todo par de pixels
- **w = 0** indica pixels idênticos (sem evidência de fronteira)
- **w grande** indica forte diferença = evidência de fronteira entre regiões

---

## 2. Predicado de Segmentação

### 2.1 Diferença Interna: Int(C)

A **diferença interna** de uma componente `C ⊆ V` é definida como o **peso máximo
da aresta** na Árvore Geradora Mínima (MST) da componente:

```
Int(C) = max   w(e)
         e ∈ MST(C, E)
```

**Intuição:** `Int(C)` representa a maior "ruptura" necessária para manter a
componente conectada. Uma componente com `Int(C)` pequeno é internamente coesa
(pixels similares). Uma componente com `Int(C)` grande apresenta alta variabilidade
interna, mas ainda é percebida como uma região única.

**Caso especial:** para uma componente com um único pixel, `|C| = 1`, a MST é
vazia e, portanto, `Int(C) = 0`.

### 2.2 Diferença entre Componentes: Dif(C₁, C₂)

A **diferença entre duas componentes** `C₁` e `C₂` é definida como o **peso
mínimo de aresta** que conecta um vértice de `C₁` a um vértice de `C₂`:

```
Dif(C₁, C₂) = min        w(v_i, v_j)
               v_i ∈ C₁, v_j ∈ C₂
               (v_i, v_j) ∈ E
```

Se não há aresta conectando `C₁` a `C₂`, então `Dif(C₁, C₂) = ∞`.

**Intuição:** `Dif(C₁, C₂)` captura a menor diferença observável entre as duas
regiões na fronteira que as separa. É a evidência mais "fraca" de fronteira. Se
mesmo a menor diferença de fronteira for grande, há evidência clara de separação.

> **Importante:** usar o mínimo em vez da mediana ou outro quantil é uma escolha
> deliberada. Mudar para um quantil torna o problema de encontrar uma boa
> segmentação NP-difícil.

### 2.3 Limiar Adaptativo: τ(C)

O **limiar adaptativo** compensa a instabilidade da estimativa de `Int(C)` para
componentes pequenas:

```
τ(C) = k / |C|
```

onde:
- `k` é uma constante de escala (parâmetro do algoritmo)
- `|C|` é o número de pixels na componente

**Intuição:** para uma componente com um único pixel, `Int(C) = 0` não é uma
estimativa confiável da variabilidade interna. O limiar `τ(C) = k/1 = k` exige
uma evidência muito forte de fronteira antes de isolar esse pixel. À medida que
a componente cresce, o limiar diminui, pois `Int(C)` passa a ser uma estimativa
mais confiável.

### 2.4 Predicado de Merge

O **predicado de comparação de regiões** `D(C₁, C₂)` avalia se há evidência de
fronteira entre `C₁` e `C₂`:

```
         ⎧ true   se  Dif(C₁, C₂) > MInt(C₁, C₂)   [há fronteira]
D(C₁,C₂)=⎨
         ⎩ false  caso contrário                       [sem fronteira → merge]
```

onde a **diferença interna mínima** é:

```
MInt(C₁, C₂) = min( Int(C₁) + τ(C₁),  Int(C₂) + τ(C₂) )
```

**Regra de merge:** dois componentes devem ser fundidos quando **não** há evidência
de fronteira, ou seja, quando:

```
Dif(C₁, C₂)  ≤  min( Int(C₁) + τ(C₁),  Int(C₂) + τ(C₂) )
```

**Intuição:** se a menor diferença entre as fronteiras das regiões for menor ou
igual à variabilidade interna de pelo menos uma das regiões (com folga τ), então
não há justificativa para mantê-las separadas.

---

## 3. Parâmetros do Algoritmo

| Parâmetro  | Descrição | Efeito |
|------------|-----------|--------|
| `k`        | Constante de escala/granularidade | `k` grande → segmentos maiores; `k` pequeno → mais fragmentos. Não é um tamanho mínimo. Pequenos segmentos são permitidos quando a diferença entre regiões é suficientemente grande. |
| `min_size` | Tamanho mínimo de componente (pós-processamento) | Componentes com menos de `min_size` pixels são fundidos com a componente vizinha mais similar após o algoritmo principal. Garante que não haja segmentos excessivamente pequenos. |
| `σ`        | Desvio padrão do filtro Gaussiano | `σ` maior → mais suavização, menos ruído, mas perda de bordas finas. `σ = 0` → sem suavização. Valor típico: `σ = 0.8`. |

**Valores típicos (conforme experimentos do artigo):** `σ = 0.8`, `k = 300`.

---

## 4. Pseudocódigo Completo

```
Algoritmo: SegmentacaoGrafo(Imagem I, σ, k, min_size)

ENTRADA:
  I        → imagem H × W (escala de cinza ou RGB)
  σ        → desvio padrão do filtro Gaussiano
  k        → constante de escala
  min_size → tamanho mínimo de segmento

SAÍDA:
  S → segmentação de V em componentes {C₁, C₂, ..., Cᵣ}

─────────────────────────────────────────────────
PRÉ-PROCESSAMENTO
─────────────────────────────────────────────────

1. I_suav ← AplicarFiltroGaussiano(I, σ)
   // Suaviza a imagem para reduzir ruído

─────────────────────────────────────────────────
CONSTRUÇÃO DO GRAFO
─────────────────────────────────────────────────

2. V ← { v_(x,y) : (x,y) ∈ I_suav }
   // Um vértice por pixel

3. E ← ∅
   Para cada pixel p = (x, y):
     Para cada vizinho q = (x', y') na vizinhança de 8:
       Se (p, q) ainda não está em E:
         w ← Dissimilaridade(I_suav[p], I_suav[q])
         E ← E ∪ { (v_p, v_q, w) }
   // Constrói arestas com pesos = diferença de intensidade/cor

─────────────────────────────────────────────────
ALGORITMO PRINCIPAL (estilo Kruskal)
─────────────────────────────────────────────────

4. π ← OrdenarArestas(E, por peso não-decrescente)
   // π = (o₁, o₂, ..., o_m) com w(o₁) ≤ w(o₂) ≤ ... ≤ w(o_m)

5. S⁰ ← { {v} : v ∈ V }
   // Segmentação inicial: cada pixel é sua própria componente
   // Inicializar Union-Find com n componentes

6. Para q = 1, 2, ..., m:
   
     oq = (v_i, v_j) ← π[q]     // aresta de índice q na ordenação
     
     C_i ← ComponenteDe(v_i, S^(q-1))
     C_j ← ComponenteDe(v_j, S^(q-1))
     
     Se C_i ≠ C_j:               // vértices em componentes distintas
       
       threshold ← min( Int(C_i) + k/|C_i|,
                        Int(C_j) + k/|C_j|  )
       
       Se w(oq) ≤ threshold:     // sem evidência de fronteira → merge
         S^q ← Merge(S^(q-1), C_i, C_j)
         Atualizar Int(C_i ∪ C_j) ← w(oq)
         // a aresta de merge é sempre o máximo da MST da nova componente
       Senão:
         S^q ← S^(q-1)           // mantém segmentação anterior
     
     Senão:
       S^q ← S^(q-1)             // mesma componente, nada a fazer

─────────────────────────────────────────────────
PÓS-PROCESSAMENTO
─────────────────────────────────────────────────

7. Para cada aresta (v_i, v_j) ∈ π (em ordem):
     C_i ← ComponenteDe(v_i, S^m)
     C_j ← ComponenteDe(v_j, S^m)
     Se C_i ≠ C_j  E  (|C_i| < min_size  OU  |C_j| < min_size):
       S^m ← Merge(S^m, C_i, C_j)

8. Retornar S^m
```

**Estrutura de dados:** utiliza-se Union-Find com compressão de caminho para
operações de merge e busca de componente em tempo quase-constante O(α(n)), onde
α é a inversa da função de Ackermann.

**Atualização de Int(C):** após cada merge de `C_i` e `C_j` pela aresta `oq`,
a diferença interna da nova componente é `Int(C_i ∪ C_j) = w(oq)`, pois as
arestas são processadas em ordem não-decrescente e a aresta de merge é sempre
o maior peso da MST do componente fundido (consequência do Lema 1 do artigo).

---

## 5. Exemplo Passo a Passo: Grade 4×4

### 5.1 Setup

Considere uma imagem em escala de cinza 4×4 com os seguintes valores de intensidade
(sem suavização, σ = 0), com parâmetros `k = 1.0` e `min_size = 2`:

```
Pixels e intensidades:

    col0  col1  col2  col3
lin0 [10]  [12]  [80]  [82]
lin1 [11]  [13]  [79]  [81]
lin2 [50]  [50]  [50]  [50]
lin3 [51]  [51]  [51]  [52]
```

Nomeamos os pixels `p00` a `p33` (linha × coluna). A imagem contém
visualmente três regiões:
- **Região A** (canto superior esquerdo): pixels escuros (~10–13)
- **Região B** (canto superior direito): pixels claros (~79–82)
- **Região C** (metade inferior): pixels de intensidade média (~50–52)

### 5.2 Construção das Arestas (vizinhança de 4 por simplicidade)

Listamos apenas as arestas horizontais e verticais com seus pesos:

```
Arestas horizontais (linha × col → col+1):
  (p00,p01): |10-12| = 2        (p01,p02): |12-80| = 68
  (p02,p03): |80-82| = 2        (p10,p11): |11-13| = 2
  (p11,p12): |13-79| = 66       (p12,p13): |79-81| = 2
  (p20,p21): |50-50| = 0        (p21,p22): |50-50| = 0
  (p22,p23): |50-50| = 0        (p30,p31): |51-51| = 0
  (p31,p32): |51-51| = 0        (p32,p33): |51-52| = 1

Arestas verticais (col × lin → lin+1):
  (p00,p10): |10-11| = 1        (p01,p11): |12-13| = 1
  (p02,p12): |80-79| = 1        (p03,p13): |82-81| = 1
  (p10,p20): |11-50| = 39       (p11,p21): |13-50| = 37
  (p12,p22): |79-50| = 29       (p13,p23): |81-50| = 31
  (p20,p30): |50-51| = 1        (p21,p31): |50-51| = 1
  (p22,p32): |50-51| = 1        (p23,p33): |50-52| = 2
```

### 5.3 Ordenação das Arestas

```
Peso 0: (p20,p21), (p21,p22), (p22,p23), (p30,p31), (p31,p32)
Peso 1: (p00,p10), (p01,p11), (p02,p12), (p03,p13),
        (p20,p30), (p21,p31), (p22,p32), (p23,p33)
Peso 2: (p00,p01), (p02,p03), (p10,p11), (p12,p13), (p23,p33)→já peso2
Peso 29: (p12,p22)
Peso 31: (p13,p23)
Peso 37: (p11,p21)
Peso 39: (p10,p20)
Peso 66: (p11,p12)
Peso 68: (p01,p02)
```

### 5.4 Estado Inicial (S⁰)

```
16 componentes singleton, uma por pixel.
Int(C) = 0 para todas.  τ(C) = k/1 = 1.0 para todas.
```

### 5.5 Iterações do Algoritmo

**Iterações com peso 0** (arestas (p20,p21), (p21,p22), (p22,p23), (p30,p31), (p31,p32)):

```
Aresta (p20,p21), w=0:
  C_i = {p20},  Int = 0,  τ = 1.0  →  threshold_i = 1.0
  C_j = {p21},  Int = 0,  τ = 1.0  →  threshold_j = 1.0
  MInt = min(1.0, 1.0) = 1.0
  w=0 ≤ 1.0  →  MERGE
  C_novo = {p20, p21},  Int = 0,  τ = k/2 = 0.5

Aresta (p21,p22), w=0:
  C_i = {p20,p21},  Int = 0,  τ = 0.5  →  threshold_i = 0.5
  C_j = {p22},      Int = 0,  τ = 1.0  →  threshold_j = 1.0
  MInt = min(0.5, 1.0) = 0.5
  w=0 ≤ 0.5  →  MERGE
  C_novo = {p20,p21,p22},  Int = 0,  τ = k/3 ≈ 0.33

Aresta (p22,p23), w=0:
  C_i = {p20,p21,p22},  threshold = 0 + 0.33 = 0.33
  C_j = {p23},          threshold = 0 + 1.0  = 1.0
  MInt = 0.33
  w=0 ≤ 0.33  →  MERGE
  C_novo = {p20,p21,p22,p23},  Int = 0,  τ = 0.25

Aresta (p30,p31), w=0:  MERGE  →  {p30,p31},  Int=0, τ=0.5
Aresta (p31,p32), w=0:  MERGE  →  {p30,p31,p32},  Int=0, τ=0.33

Estado após peso 0:
  A  = {p00}, {p01}, {p02}, {p03}  (4 singletons)
  B  = {p10}, {p11}, {p12}, {p13}  (4 singletons)
  C1 = {p20,p21,p22,p23}           Int=0, |C|=4
  C2 = {p30,p31,p32}               Int=0, |C|=3
  C3 = {p33}                        singleton
```

**Iterações com peso 1** (arestas intra-região A, B e linhas 2→3):

```
Aresta (p00,p10), w=1:
  C_i = {p00}, threshold = 0 + 1.0 = 1.0
  C_j = {p10}, threshold = 0 + 1.0 = 1.0
  MInt = 1.0,  w=1 ≤ 1.0  →  MERGE  →  {p00,p10}, Int=1, τ=0.5

Aresta (p01,p11), w=1:
  MERGE  →  {p01,p11}, Int=1, τ=0.5

Aresta (p02,p12), w=1:
  MERGE  →  {p02,p12}, Int=1, τ=0.5

Aresta (p03,p13), w=1:
  MERGE  →  {p03,p13}, Int=1, τ=0.5

Aresta (p20,p30), w=1:
  C_i = {p20,p21,p22,p23},  Int=0, τ=0.25  →  threshold=0.25
  C_j = {p30,p31,p32},      Int=0, τ=0.33  →  threshold=0.33
  MInt = min(0.25, 0.33) = 0.25
  w=1 > 0.25  →  NÃO MERGE

Aresta (p21,p31), w=1:  mesmas componentes acima  →  NÃO MERGE
Aresta (p22,p32), w=1:  idem  →  NÃO MERGE

Aresta (p23,p33), w=2 (na verdade peso 2, avançamos):
  (processado na próxima rodada)
```

**Iterações com peso 2** (arestas intra-região A e B):

```
Aresta (p00,p01), w=2:
  C_i = {p00,p10},  Int=1, τ=0.5  →  threshold=1.5
  C_j = {p01,p11},  Int=1, τ=0.5  →  threshold=1.5
  MInt = 1.5,  w=2 > 1.5  →  NÃO MERGE

Aresta (p02,p03), w=2:
  C_i = {p02,p12},  threshold=1.5
  C_j = {p03,p13},  threshold=1.5
  MInt = 1.5,  w=2 > 1.5  →  NÃO MERGE

Aresta (p10,p11), w=2:
  C_i = {p00,p10},  threshold=1.5
  C_j = {p01,p11},  threshold=1.5
  MInt = 1.5,  w=2 > 1.5  →  NÃO MERGE

Aresta (p12,p13), w=2:
  idem  →  NÃO MERGE

Aresta (p23,p33), w=2:
  C_i = {p20,p21,p22,p23},  Int=0, τ=0.25  →  threshold=0.25
  C_j = {p33},               Int=0, τ=1.0   →  threshold=1.0
  MInt = min(0.25, 1.0) = 0.25
  w=2 > 0.25  →  NÃO MERGE
```

**Iterações com pesos 29, 31, 37, 39** (fronteira entre linhas 1 e 2):

```
Todos esses pesos são muito maiores que os thresholds disponíveis
(componentes da linha 2 têm Int=0, |C|=4, threshold=0.25).
  →  NÃO MERGE em nenhum caso.
```

**Iterações com pesos 66 e 68** (fronteira A-B):

```
Aresta (p11,p12), w=66:
  C_i = {p01,p11},  Int=1, τ=0.5  →  threshold=1.5
  C_j = {p02,p12},  Int=1, τ=0.5  →  threshold=1.5
  MInt=1.5,  w=66 >> 1.5  →  NÃO MERGE

Aresta (p01,p02), w=68:
  idem  →  NÃO MERGE
```

### 5.6 Resultado após Algoritmo Principal

```
Componentes antes do pós-processamento:

  Região A (superior esquerda):
    {p00,p10},  {p01,p11},  →  (k pequeno não fundiu os pares)

  Região B (superior direita):
    {p02,p12},  {p03,p13}

  Região C (inferior):
    {p20,p21,p22,p23},  {p30,p31,p32},  {p33}
```

### 5.7 Pós-processamento (min_size = 4)

Componentes com menos de 4 pixels são fundidas com a vizinha de aresta mínima:

```
{p00,p10} (|C|=2) → fundida com {p01,p11} via aresta de peso 2
  → {p00,p10,p01,p11}

{p02,p12} (|C|=2) → fundida com {p03,p13}
  → {p02,p12,p03,p13}

{p30,p31,p32} (|C|=3) → fundida com {p20,...,p23} via aresta de peso 1
  → {p20,p21,p22,p23,p30,p31,p32}

{p33} (|C|=1) → fundida com componente vizinha de menor aresta
  → integrada à componente C inferior
```

### 5.8 Segmentação Final

```
┌──────┬──────┬──────┬──────┐
│  A   │  A   │  B   │  B   │
│ p00  │ p01  │ p02  │ p03  │
├──────┼──────┼──────┼──────┤
│  A   │  A   │  B   │  B   │
│ p10  │ p11  │ p12  │ p13  │
├──────┼──────┼──────┼──────┤
│  C   │  C   │  C   │  C   │
│ p20  │ p21  │ p22  │ p23  │
├──────┼──────┼──────┼──────┤
│  C   │  C   │  C   │  C   │
│ p30  │ p31  │ p32  │ p33  │
└──────┴──────┴──────┴──────┘

Região A = pixels com intensidade ~10–13  (escuro)
Região B = pixels com intensidade ~79–82  (claro)
Região C = pixels com intensidade ~50–52  (médio)
```

As três regiões perceptualmente distintas foram corretamente identificadas.

---

## 6. Análise de Complexidade

### 6.1 Complexidade Total: O(m log m)

A complexidade do algoritmo é **O(m log m)**, onde `m` é o número de arestas do grafo.

Para imagens de `n` pixels com vizinhança de 4 ou 8, `m = O(n)`, portanto a complexidade
é equivalente a **O(n log n)**.

### 6.2 Decomposição por Etapa

| Etapa | Operação | Complexidade |
|-------|----------|--------------|
| Pré-processamento | Filtro Gaussiano | O(n) |
| Construção do grafo | Criar V e E | O(n) |
| Ordenação das arestas | QuickSort / MergeSort | **O(m log m)** ← dominante |
| Inicialização Union-Find | n componentes singleton | O(n) |
| Loop principal | m iterações × O(α(n)) por Union-Find | O(m α(n)) ≈ O(m) |
| Atualização de Int(C) | Constante por merge | O(m) |
| Pós-processamento | Percorre E novamente | O(m) |
| **Total** | | **O(m log m)** |

onde `α(n)` é a inversa da função de Ackermann, efetivamente constante.

### 6.3 Por que O(m log m) e não O(m)?

O gargalo é a **ordenação das arestas** no passo 4. Se as arestas já estivessem
ordenadas (por exemplo, para imagens de 8 bits onde os pesos são inteiros de 0 a 255),
poderíamos usar Counting Sort em O(m + W) onde W = 255, reduzindo a complexidade
total para **O(n)** na prática.

### 6.4 Comparação com Outros Métodos

| Método | Complexidade |
|--------|--------------|
| Felzenszwalb-Huttenlocher (este) | O(m log m) ≈ O(n log n) |
| Normalized Cuts (Shi & Malik) | O(n²) a O(n³) na prática |
| Mean Shift | O(n² ) |
| K-Means (k iterações) | O(k·n·i) |

O algoritmo é **praticamente linear**, capaz de processar imagens 320×240 em
fração de segundo, viabilizando uso em vídeo em tempo real.

---

## 7. Propriedades Globais Garantidas

Apesar de ser um algoritmo guloso (greedy), o algoritmo garante que a segmentação
produzida S satisfaz simultaneamente:

**Não é muito fina:** para todo par de componentes `C₁, C₂ ∈ S` adjacentes,
há evidência de fronteira entre eles:
```
Dif(C₁, C₂) > MInt(C₁, C₂)
```

**Não é muito grossa:** não existe refinamento próprio de S que também seja
"não muito fino". Em outras palavras, não é possível subdividir nenhuma região
e ainda ter evidência de fronteira entre todos os pares vizinhos resultantes.

Essas propriedades são análogas às garantias globais dos métodos de corte de
grafo (como Normalized Cuts), mas obtidas com complexidade computacional muito menor.

---

## Referências

Felzenszwalb, P. F., & Huttenlocher, D. P. (2004).
*Efficient Graph-Based Image Segmentation.*
International Journal of Computer Vision, 59(2), 167–181.

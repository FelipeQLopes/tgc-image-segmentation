# Segmentação de Imagens Baseada em Grafos
Projeto desenvolvido para a disciplina de Teoria de Grafos e Computabilidade, com o objetivo de implementar e analisar algoritmos de segmentação de imagens baseados em Grafos, Árvores Geradoras Mínimas (MST) e Caminho Mínimo, aplicados a imagens em tons de cinza e coloridas.


## Integrantes:
- Arthur Camargo
- Daniel Matos
- Davi Bernardes
- Felipe Costa
- Felipe Quites
- Leonardo Amaral
- Matheus de Almeida
- Nayron Campos

## Como Compilar e Executar

### Compilação

Para compilar o projeto, execute:

```bash
make
```

ou

```bash
make all
```

Durante a compilação:

1. A pasta `build` será criada automaticamente caso não exista.
2. Os arquivos `.cpp` serão compilados para arquivos objeto (`.o`).
3. O executável final será gerado na pasta `build`.


---

### Execução

Após a compilação, execute:

```bash
make run
```

ou diretamente:

```bash
./build/programa
```


---

### Limpeza dos Arquivos Gerados

Para remover todos os arquivos produzidos durante a compilação:

```bash
make clean
```

Este comando remove completamente a pasta `build` e todos os arquivos gerados.


---

### Fluxo de Uso Recomendado

```bash
# Compilar o projeto
make

# Executar o programa
make run

# Limpar arquivos gerados
make clean
```
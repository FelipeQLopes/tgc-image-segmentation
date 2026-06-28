TARGET = build/main
TEST_TARGET = build/test_image
TEST_GRAPH_TARGET = build/test_graph
TEST_DS_TARGET = build/test_disjoint_set
TEST_KRUSKAL_TARGET = build/test_kruskal
TEST_PQ_TARGET = build/test_priority_queue
TEST_HIERARCHY_TARGET = build/test_hierarchy
TEST_COUSTY_TARGET = build/test_cousty
TEST_GRADIENT_TARGET = build/test_gradient
TEST_FELZENSZWALB_TARGET = build/test_felzenszwalb

# Diretorios
SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = tests

# Compilador e Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -O0 -I$(INCDIR)

# Encontra todos os arquivos .cpp na pasta src
SOURCES := $(wildcard $(SRCDIR)/*.cpp)

# Define os arquivos de objeto (.o) correspondentes dentro da pasta build
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Regra padrao
all: $(BUILDDIR) $(TARGET)

# Linkagem
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Compilacao
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Garante que build existe
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Executa
run: all
	./$(TARGET)

# Testes

test: test_ds test_image test_graph test_kruskal test_pq test_hierarchy test_cousty test_gradient test_felzenszwalb

test_ds: $(BUILDDIR) $(TEST_DS_TARGET)
	./$(TEST_DS_TARGET)

test_image: $(BUILDDIR) $(TEST_TARGET)
	./$(TEST_TARGET)

test_graph: $(BUILDDIR) $(TEST_GRAPH_TARGET)
	./$(TEST_GRAPH_TARGET)

test_kruskal: $(BUILDDIR) $(TEST_KRUSKAL_TARGET)
	./$(TEST_KRUSKAL_TARGET)

test_pq: $(BUILDDIR) $(TEST_PQ_TARGET)
	./$(TEST_PQ_TARGET)

test_hierarchy: $(BUILDDIR) $(TEST_HIERARCHY_TARGET)
	./$(TEST_HIERARCHY_TARGET)

test_cousty: $(BUILDDIR) $(TEST_COUSTY_TARGET)
	./$(TEST_COUSTY_TARGET)

test_gradient: $(BUILDDIR) $(TEST_GRADIENT_TARGET)
	./$(TEST_GRADIENT_TARGET)

test_felzenszwalb: $(BUILDDIR) $(TEST_FELZENSZWALB_TARGET)
	./$(TEST_FELZENSZWALB_TARGET)

$(TEST_DS_TARGET): $(TESTDIR)/test_disjoint_set.cpp $(SRCDIR)/DisjointSet.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/DisjointSet.cpp $(TESTDIR)/test_disjoint_set.cpp -o $@ -lm

$(TEST_TARGET): $(TESTDIR)/test_image.cpp $(SRCDIR)/Image.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(TESTDIR)/test_image.cpp -o $@ -lm

$(TEST_GRAPH_TARGET): $(TESTDIR)/test_graph.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(TESTDIR)/test_graph.cpp -o $@ -lm

$(TEST_KRUSKAL_TARGET): $(TESTDIR)/test_kruskal.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(TESTDIR)/test_kruskal.cpp -o $@ -lm

$(TEST_PQ_TARGET): $(TESTDIR)/test_priority_queue.cpp $(SRCDIR)/PriorityQueue.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/PriorityQueue.cpp $(TESTDIR)/test_priority_queue.cpp -o $@

$(TEST_HIERARCHY_TARGET): $(TESTDIR)/test_hierarchy.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(SRCDIR)/Hierarchy.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(SRCDIR)/Hierarchy.cpp $(TESTDIR)/test_hierarchy.cpp -o $@ -lm

$(TEST_COUSTY_TARGET): $(TESTDIR)/test_cousty.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(SRCDIR)/Hierarchy.cpp $(SRCDIR)/SaliencyMap.cpp $(SRCDIR)/Cousty.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(SRCDIR)/Hierarchy.cpp $(SRCDIR)/SaliencyMap.cpp $(SRCDIR)/Cousty.cpp $(TESTDIR)/test_cousty.cpp -o $@ -lm

$(TEST_GRADIENT_TARGET): $(TESTDIR)/test_gradient.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Gradient.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Gradient.cpp $(TESTDIR)/test_gradient.cpp -o $@ -lm

$(TEST_FELZENSZWALB_TARGET): $(TESTDIR)/test_felzenszwalb.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Felzenszwalb.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Felzenszwalb.cpp $(TESTDIR)/test_felzenszwalb.cpp -o $@ -lm

# Limpa
clean:
	rm -rf $(BUILDDIR)


.PHONY: all run clean test test_ds test_image test_graph test_kruskal test_pq test_hierarchy test_cousty test_gradient test_felzenszwalb
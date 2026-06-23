TARGET = build/main
TEST_TARGET = build/test_image
TEST_GRAPH_TARGET = build/test_graph
TEST_DS_TARGET = build/test_disjoint_set
TEST_KRUSKAL_TARGET = build/test_kruskal
TEST_PQ_TARGET = build/test_priority_queue

# Diretorios
SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = tests

# Compilador e Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I$(INCDIR)

# Encontra todos os arquivos .cpp na pasta src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

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
test: test_ds test_image test_graph test_kruskal test_pq

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

$(TEST_DS_TARGET): $(TESTDIR)/test_disjoint_set.cpp $(SRCDIR)/DisjointSet.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/DisjointSet.cpp $(TESTDIR)/test_disjoint_set.cpp -o $@ -lm

$(TEST_TARGET): $(TESTDIR)/test_image.cpp $(SRCDIR)/Image.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(TESTDIR)/test_image.cpp -o $@ -lm

$(TEST_GRAPH_TARGET): $(TESTDIR)/test_graph.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(TESTDIR)/test_graph.cpp -o $@ -lm

$(TEST_KRUSKAL_TARGET): $(TESTDIR)/test_kruskal.cpp $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/DisjointSet.cpp $(SRCDIR)/Kruskal.cpp $(TESTDIR)/test_kruskal.cpp -o $@ -lm

$(TEST_PQ_TARGET): $(TESTDIR)/teste_priority_queue.cpp $(SRCDIR)/PriorityQueue.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/PriorityQueue.cpp $(TESTDIR)/teste_priority_queue.cpp -o $@

# Limpa
clean:
	rm -rf $(BUILDDIR)

.PHONY: all run clean test test_ds test_image test_graph test_kruskal test_pq

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

# Encontra todos os arquivos .cpp na pasta src e subpastas
SOURCES := $(wildcard $(SRCDIR)/*.cpp) \
           $(wildcard $(SRCDIR)/core/*.cpp) \
           $(wildcard $(SRCDIR)/graph/*.cpp) \
           $(wildcard $(SRCDIR)/image_processing/*.cpp) \
           $(wildcard $(SRCDIR)/hierarchy/*.cpp) \
           $(wildcard $(SRCDIR)/algorithms/*.cpp)

ifeq ($(OS),Windows_NT)
    MKDIR = if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"
    MKDIR_BUILD = if not exist "$(subst /,\,$(BUILDDIR))" mkdir "$(subst /,\,$(BUILDDIR))"
    RMDIR = if exist "$(subst /,\,$(BUILDDIR))" rmdir /s /q "$(subst /,\,$(BUILDDIR))"
else
    MKDIR = mkdir -p $(@D)
    MKDIR_BUILD = mkdir -p $(BUILDDIR)
    RMDIR = rm -rf $(BUILDDIR)
endif

# Define os arquivos de objeto (.o) correspondentes dentro da pasta build
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Regra padrao
all: $(BUILDDIR) $(TARGET)

# Linkagem
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Compilacao (cria subdiretorios no build automaticamente)
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Garante que build existe
$(BUILDDIR):
	@$(MKDIR_BUILD)

# Executa
run: all
	./$(TARGET)

# Atalhos para caminhos de source
SRC_CORE = $(SRCDIR)/core
SRC_GRAPH = $(SRCDIR)/graph
SRC_IMGPROC = $(SRCDIR)/image_processing
SRC_HIERARCHY = $(SRCDIR)/hierarchy
SRC_ALGO = $(SRCDIR)/algorithms

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

$(TEST_DS_TARGET): $(TESTDIR)/test_disjoint_set.cpp $(SRC_GRAPH)/DisjointSet.cpp
	$(CXX) $(CXXFLAGS) $(SRC_GRAPH)/DisjointSet.cpp $(TESTDIR)/test_disjoint_set.cpp -o $@ -lm

$(TEST_TARGET): $(TESTDIR)/test_image.cpp $(SRC_CORE)/Image.cpp
	$(CXX) $(CXXFLAGS) $(SRC_CORE)/Image.cpp $(TESTDIR)/test_image.cpp -o $@ -lm

$(TEST_GRAPH_TARGET): $(TESTDIR)/test_graph.cpp $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp
	$(CXX) $(CXXFLAGS) $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp $(TESTDIR)/test_graph.cpp -o $@ -lm

$(TEST_KRUSKAL_TARGET): $(TESTDIR)/test_kruskal.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp
	$(CXX) $(CXXFLAGS) $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp $(TESTDIR)/test_kruskal.cpp -o $@ -lm

$(TEST_PQ_TARGET): $(TESTDIR)/test_priority_queue.cpp $(SRC_IMGPROC)/PriorityQueue.cpp
	$(CXX) $(CXXFLAGS) $(SRC_IMGPROC)/PriorityQueue.cpp $(TESTDIR)/test_priority_queue.cpp -o $@

$(TEST_HIERARCHY_TARGET): $(TESTDIR)/test_hierarchy.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp $(SRC_HIERARCHY)/Hierarchy.cpp
	$(CXX) $(CXXFLAGS) $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp $(SRC_HIERARCHY)/Hierarchy.cpp $(TESTDIR)/test_hierarchy.cpp -o $@ -lm

$(TEST_COUSTY_TARGET): $(TESTDIR)/test_cousty.cpp $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp $(SRC_HIERARCHY)/Hierarchy.cpp $(SRC_HIERARCHY)/SaliencyMap.cpp $(SRC_ALGO)/Cousty.cpp
	$(CXX) $(CXXFLAGS) $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_GRAPH)/Kruskal.cpp $(SRC_HIERARCHY)/Hierarchy.cpp $(SRC_HIERARCHY)/SaliencyMap.cpp $(SRC_ALGO)/Cousty.cpp $(TESTDIR)/test_cousty.cpp -o $@ -lm

$(TEST_GRADIENT_TARGET): $(TESTDIR)/test_gradient.cpp $(SRC_CORE)/Image.cpp $(SRC_IMGPROC)/Gradient.cpp
	$(CXX) $(CXXFLAGS) $(SRC_CORE)/Image.cpp $(SRC_IMGPROC)/Gradient.cpp $(TESTDIR)/test_gradient.cpp -o $@ -lm

$(TEST_FELZENSZWALB_TARGET): $(TESTDIR)/test_felzenszwalb.cpp $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_ALGO)/Felzenszwalb.cpp
	$(CXX) $(CXXFLAGS) $(SRC_CORE)/Image.cpp $(SRC_GRAPH)/Graph.cpp $(SRC_GRAPH)/DisjointSet.cpp $(SRC_ALGO)/Felzenszwalb.cpp $(TESTDIR)/test_felzenszwalb.cpp -o $@ -lm

# Limpa
clean:
	@$(RMDIR)


.PHONY: all run clean test test_ds test_image test_graph test_kruskal test_pq test_hierarchy test_cousty test_gradient test_felzenszwalb
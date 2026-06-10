TARGET = build/main
TEST_TARGET = build/test_image
TEST_GRAPH_TARGET = build/test_graph

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
test: test_image test_graph

test_image: $(BUILDDIR) $(TEST_TARGET)
	./$(TEST_TARGET)

test_graph: $(BUILDDIR) $(TEST_GRAPH_TARGET)
	./$(TEST_GRAPH_TARGET)

$(TEST_TARGET): $(TESTDIR)/test_image.cpp $(SRCDIR)/Image.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(TESTDIR)/test_image.cpp -o $@ -lm

$(TEST_GRAPH_TARGET): $(TESTDIR)/test_graph.cpp $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp
	$(CXX) $(CXXFLAGS) $(SRCDIR)/Image.cpp $(SRCDIR)/Graph.cpp $(TESTDIR)/test_graph.cpp -o $@ -lm

# Limpa
clean:
	rm -rf $(BUILDDIR)

.PHONY: all run clean test test_image test_graph
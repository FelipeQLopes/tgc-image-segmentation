TARGET = build/main

# Diretórios
SRCDIR = src
INCDIR = include
BUILDDIR = build

# Compilador e Flags
CXX = g++
CXXFLAGS = -Wall -Wextra -I$(INCDIR)

# Encontra todos os arquivos .cpp na pasta src
SOURCES = $(wildcard $(SRCDIR)/*.cpp)

# Define os arquivos de objeto (.o) correspondentes dentro da pasta build
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

# Regra padrão
all: $(BUILDDIR) $(TARGET)

# Linkagem
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

# Compilação
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Garante que build existe
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Executa
run: all
	./$(TARGET)

# Limpa
clean:
	rm -rf $(BUILDDIR)

.PHONY: all run clean
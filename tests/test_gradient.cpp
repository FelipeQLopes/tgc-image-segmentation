#include "Gradient.hpp"

#include "image.hpp"

#include <cassert>
#include <iostream>
#include <set>

static int tests_passed = 0;

#define TEST(name)                                                             \
    do {                                                                       \
        std::cout << "  [RUN ] " << (name) << std::endl;                       \
    } while (0)

#define PASS(name)                                                             \
    do {                                                                       \
        std::cout << "  [PASS] " << (name) << std::endl;                       \
        ++tests_passed;                                                        \
    } while (0)

static void test_gradient_on_rectangle() {
    const char* name = "gradient: retangulo branco sobre fundo preto";
    TEST(name);

    int w = 10, h = 10;
    Image img(w, h, 1);
    // background 0
    // retangulo x=3..6, y=3..6 set to 255
    for (int y = 3; y <= 6; ++y) for (int x = 3; x <= 6; ++x) img.set(x,y,0,255);

    Image g = compute_gradient(img);

    // centro do background deve ser zero
    assert(g.at(0,0,0) == 0);
    // centro do retangulo deve ser zero
    assert(g.at(4,4,0) == 0);
    // pixel de borda deve ter gradiente > 0
    bool any_edge_nonzero = false;
    for (int y = 3; y <= 6 && !any_edge_nonzero; ++y) {
        if (g.at(3,y,0) != 0) any_edge_nonzero = true;
        if (g.at(6,y,0) != 0) any_edge_nonzero = true;
    }
    for (int x = 3; x <= 6 && !any_edge_nonzero; ++x) {
        if (g.at(x,3,0) != 0) any_edge_nonzero = true;
        if (g.at(x,6,0) != 0) any_edge_nonzero = true;
    }
    assert(any_edge_nonzero);

    PASS(name);
}

static void test_regional_minima_on_rectangle() {
    const char* name = "minimos regionais detectados em regioes uniformes";
    TEST(name);

    int w = 10, h = 10;
    Image img(w, h, 1);
    for (int y = 3; y <= 6; ++y) for (int x = 3; x <= 6; ++x) img.set(x,y,0,255);

    Image g = compute_gradient(img);
    auto labels = find_regional_minima(g);

    // conta os labels unicos encontrados (excluindo -1)
    std::set<int> uniq;
    for (int v : labels) if (v >= 0) uniq.insert(v);

    // espera pelo menos 2 minimos: regiao de fundo e regiao do retangulo
    assert(static_cast<int>(uniq.size()) >= 2);

    // garante que pixel interno do retangulo tem label >=0
    int label_rect = labels[4 + 4*w];
    assert(label_rect >= 0);

    // garante que pixel interno do background tem label diferente
    int label_bg = labels[0 + 0*w];
    assert(label_bg >= 0);
    assert(label_bg != label_rect);

    // garante que pixel de borda nao tenha label
    assert(labels[3 + 3*w] == -1 || labels[3 + 4*w] == -1 || labels[2 + 2*w] == -1);

    PASS(name);
}

int main() {
    test_gradient_on_rectangle();
    test_regional_minima_on_rectangle();

    std::cout << "\nTests passed: " << tests_passed << "\n";
    return 0;
}

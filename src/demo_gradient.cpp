#include "Gradient.hpp"
#include "image.hpp"

#include <iostream>
#include <string>

// Helper: cria imagem sintética com retângulos
static Image create_synthetic_image() {
    int w = 200, h = 200;
    Image img(w, h, 1);

    // Fundo preto
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.set(x, y, 0, 0);
        }
    }

    // Retângulo branco (50, 50) -> (150, 150)
    for (int y = 50; y < 150; ++y) {
        for (int x = 50; x < 150; ++x) {
            img.set(x, y, 0, 255);
        }
    }

    // Retângulo cinza (80, 80) -> (120, 120)
    for (int y = 80; y < 120; ++y) {
        for (int x = 80; x < 120; ++x) {
            img.set(x, y, 0, 128);
        }
    }

    return img;
}

int main() {
    std::cout << "=== Demo: Gradiente e Minimos Regionais ===\n\n";

    // 1. Criar/carregar imagem
    std::cout << "[1] Criando imagem sintética...\n";
    Image img = create_synthetic_image();
    std::cout << "    Imagem: " << img.width << "x" << img.height << " (1 canal)\n";

    // 2. Salvar imagem original
    std::cout << "[2] Salvando imagem original...\n";
    save_image("output/01_original.png", img);
    std::cout << "    -> output/01_original.png\n";

    // 3. Calcular gradiente
    std::cout << "[3] Calculando gradiente morfologico...\n";
    Image gradient = compute_gradient(img);
    std::cout << "    Gradiente: " << gradient.width << "x" << gradient.height
              << " (1 canal)\n";

    // 4. Salvar mapa de gradiente
    std::cout << "[4] Salvando mapa de gradiente...\n";
    save_image("output/02_gradient.png", gradient);
    std::cout << "    -> output/02_gradient.png\n";

    // 5. Detectar mínimos regionais
    std::cout << "[5] Detectando minimos regionais...\n";
    auto minima_labels = find_regional_minima(gradient);

    // Contar mínimos únicos
    int max_label = -1;
    for (int v : minima_labels) {
        if (v > max_label) max_label = v;
    }
    std::cout << "    Detectados " << (max_label + 1)
              << " minimos regionais\n";

    // 6. Colorizar e salvar mapa de mínimos
    std::cout << "[6] Colorindo mapa de minimos...\n";
    Image minima_colored = colorize_segmentation(minima_labels, gradient.width,
                                                  gradient.height);
    save_image("output/03_minima_colored.png", minima_colored);
    std::cout << "    -> output/03_minima_colored.png\n";

    std::cout << "\n=== Concluido! ===\n";
    std::cout << "Arquivos gerados em output/:\n";
    std::cout << "  - 01_original.png       (imagem de entrada)\n";
    std::cout << "  - 02_gradient.png       (mapa de gradiente)\n";
    std::cout << "  - 03_minima_colored.png (minimos regionais coloridos)\n";
    std::cout << "\nAbra as imagens para visualizar os resultados.\n";

    return 0;
}

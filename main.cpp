#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

#include "../inc/graphDir.hpp"
#include <unordered_map>
#include <array>
#include <cstdlib> 

double euclidiana(unsigned char *img, int i, int j) {
    int ri = img[i * 3 + 0];
    int gi = img[i * 3 + 1];
    int bi = img[i * 3 + 2];

    int rj = img[j * 3 + 0];
    int gj = img[j * 3 + 1];
    int bj = img[j * 3 + 2];

    double dist = std::sqrt(
        (ri - rj) * (ri - rj) +
        (gi - gj) * (gi - gj) +
        (bi - bj) * (bi - bj)
    );

    return dist;
}

int main(void) { 
    
     int width=2, height=2, channels = 3;
     std::string im = "imgs/64x64.png";
    unsigned char *img =
        stbi_load(im.c_str(), &width, &height, &channels, 3);
    
    
    //retorna a imagem convertida em super array
    /*
    for(int i=0;i<width*height*3;i++){
        std::cout << "[" << (int)img[i] << "] ";
    }
    std::cout << '\n';*/
    //cada vértice é de 3 em 3
    double maxDist = 0;
    for(int i = 0; i < width*height*3; i += 3) {
        if((i/3)%width > 0) maxDist = std::max(maxDist, euclidiana(img, i/3, i/3-1));
        if(i/3/width > 0) maxDist = std::max(maxDist, euclidiana(img, i/3, i/3-width));
    }
    double gamma = maxDist * 1.5;
    
    Graph image = Graph(width*height);
    for(int i=0;i<width*height*3;i=i+3){
        if((i/3)%width>0 ){//n é na esq
            double dist = euclidiana(img, i/3, i/3-1);
            image.addEdge(i/3,i/3-1,dist);
            image.addEdge(i/3-1,i/3,dist);
        }
        if(i/3/width>0){//n é em cima
            double dist = euclidiana(img, i/3, i/3-width);
            image.addEdge(i/3,i/3-width,dist);
            image.addEdge(i/3-width,i/3,dist);
        }
    }

    std::cout << "Image: \n" << image.toString() << std::endl;

    //adiciono vértice ligando a todo mundo:
    Graph image2 = Graph(width*height+1);

    for (int u = 0; u < width*height; u++) {
        for (auto &e : image.listaAdjacencia[u]) {
            image2.addEdge(e.u, e.v, e.w);
        }
    }

    // adiciona o super root
    for (int i = 0; i < width*height; i++) {
        image2.addEdge(width*height, i, gamma);
    }

    std::cout << "Image2: \n" << image2.toString() << std::endl;

    std::cout << "abc";

    //Graph gab = image2.optimumBranchingGabow(width*height);
    Graph gab = image2.optimumBranchingTarjan(width*height);
    std::cout << "MSA: \n" << gab.toString() << std::endl;

    Graph image3 = Graph(width*height);
    for (int u = 0; u < width*height; u++) {
        for (auto &e : gab.listaAdjacencia[u]) {
            image3.addEdge(e.u, e.v, e.w);
        }
    }

    std::cout << "Image3: \n" << image3.toString() << std::endl;

    std::vector<int> fracamente;
    fracamente = image3.componentesFracamenteConexos();

    for (int idx = 0; idx < fracamente.size(); idx++) {
        std::cout << fracamente[idx] << " ";
    }

    struct Acc {
        long r = 0, g = 0, b = 0;
        int count = 0;
    };

    std::unordered_map<int, Acc> acc;

    for (int i = 0; i < width * height; i++) {
        int comp = fracamente[i];

        acc[comp].r += img[i*3 + 0];
        acc[comp].g += img[i*3 + 1];
        acc[comp].b += img[i*3 + 2];
        acc[comp].count++;
    }

    std::unordered_map<int, std::array<unsigned char,3>> corMedia;

    for (auto &p : acc) {
        corMedia[p.first] = {
            (unsigned char)(p.second.r / p.second.count),
            (unsigned char)(p.second.g / p.second.count),
            (unsigned char)(p.second.b / p.second.count)
        };
    }

    unsigned char* out =
        (unsigned char*)malloc(width * height * 3);

    for (int i = 0; i < width * height; i++) {
        auto c = corMedia[fracamente[i]];

        out[i*3 + 0] = c[0];
        out[i*3 + 1] = c[1];
        out[i*3 + 2] = c[2];
    }

    stbi_write_png(
        "imgs/segmentada_media.png",
        width,
        height,
        3,
        out,
        width * 3
    );

    free(out);





    stbi_image_free(img);

    return 0;
}

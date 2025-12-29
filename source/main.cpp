//
// Created by Saman on 29.12.25.
//

#include <cstdio>

#include "multistage_tree.h"

int main() {
    printf("Hello, World!");
    MSTree irradianceTree{};

    // Populate MSTree with random values
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<float> randDistr(0.f, 1.f);
    for (int i = 0; i < 64; ++i) {
        irradianceTree.add(randDistr(randEng), randDistr(randEng));
    }
    irradianceTree.compile();
    // Export to csv
    irradianceTree.exportToCsv("mstree.csv");

    return 0;
}

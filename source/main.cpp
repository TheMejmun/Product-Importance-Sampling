//
// Created by Saman on 29.12.25.
//

#include <cstdio>

#include "brdf.h"
#include "diffuse_brdf.h"
#include "light_source.h"
#include "multistage_tree.h"

constexpr float M_PI_F = M_PI;
constexpr uint32_t LIGHT_SOURCES = 4;
constexpr float LIGHT_MAX_INTENSITY = 1.0f;
constexpr uint32_t MS_TREE_SAMPLES = 64;

std::random_device randDev;
std::default_random_engine randEng(randDev());
std::uniform_real_distribution<float> randDistr(0.f, 1.f);

LightSource generateLightSource() {
    const float startAngle = randDistr(randEng) * M_PI_F;
    const float endAngle = randDistr(randEng) * (M_PI_F - startAngle) + startAngle;
    const float intensity = randDistr(randEng);
    return {startAngle, endAngle, intensity};
}

MSTree setupIrradianceTree(const std::vector<LightSource> &lightSources) {
    MSTree irradianceTree{};

    // Populate MSTree with random values
    for (int i = 0; i < MS_TREE_SAMPLES; ++i) {
        // float angle = randDistr(randEng) * M_PI_F;
        // if
        irradianceTree.add(randDistr(randEng), randDistr(randEng));
    }
    irradianceTree.compile();
    // Export to csv
    irradianceTree.exportToCsv("mstree.csv");
    return irradianceTree;
}

int main() {
    printf("Hello, World!");
    DiffuseBRDF diffuse{};
    std::vector<LightSource> lightSources(LIGHT_SOURCES);
    for (uint32_t i = 0; i < LIGHT_SOURCES; ++i)
        lightSources[i] = generateLightSource();
    MSTree irradianceTree = setupIrradianceTree(lightSources);


    return 0;
}

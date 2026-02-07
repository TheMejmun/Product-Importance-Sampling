//
// Created by Saman on 29.12.25.
//

#include <cstdio>
#include <iostream>
#include <ostream>

#include "brdf/brdf.h"
#include "renderers/brdf_sampling.h"
#include "brdf/diffuse_brdf.h"
#include "light_source.h"
#include "multistage_tree.h"
#include "renderers/mstree_sampling.h"

constexpr float M_PI_F = M_PI;
constexpr uint32_t LIGHT_SOURCES = 4;
constexpr float LIGHT_MAX_INTENSITY = 100.0f;
constexpr uint32_t MS_TREE_SAMPLES = 1024;

constexpr uint32_t REFERENCE_SAMPLES = 1048576; // 2 ^ 20
constexpr uint32_t BENCHMARK_SAMPLES = 64;

// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<float> randDistr(0.f, 1.f);
    std::uniform_int_distribution<uint32_t> randIntDistr(0, std::numeric_limits<uint32_t>::max());
}

LightSource generateLightSource() {
    const float startAngle = randDistr(randEng) * M_PI_F;
    const float endAngle = randDistr(randEng) * (M_PI_F - startAngle) + startAngle;
    const float intensity = randDistr(randEng);
    return {startAngle, endAngle, intensity};
}

float calculateAnalyticalIrradiance(const std::vector<LightSource> &lightSources) {
    float irradiance = 0.f;
    for (const auto &light: lightSources) {
        irradiance += light.intensity * (light.end_angle - light.start_angle) / M_PI_F;
    }
    return irradiance;
}

MSTree setupIrradianceTree(const std::vector<LightSource> &lightSources) {
    MSTree irradianceTree{};

    for (int i = 0; i < MS_TREE_SAMPLES; ++i) {
        float angle = randDistr(randEng) * M_PI_F;

        // Get light sources that can be reached in that direction
        std::vector<LightSource const *> reachableLights;
        for (const auto &light: lightSources) {
            if (light.start_angle <= angle && light.end_angle >= angle) {
                reachableLights.push_back(&light);
            }
        }
        // Randomly take one of them for the intensity
        float intensity = reachableLights.empty()
                              ? 0
                              : reachableLights[randIntDistr(randEng) % reachableLights.size()]->intensity;
        // std::cout << "lightSources: " << reachableLights.size() << ", selected intensity " << intensity << std::endl;
        irradianceTree.add(angle, intensity);
    }
    irradianceTree.compile();
    // Export to csv
    irradianceTree.exportToCsv("mstree.csv");
    return irradianceTree;
}

int main() {
    DiffuseBRDF diffuse{};
    std::vector<LightSource> lightSources(LIGHT_SOURCES);
    for (uint32_t i = 0; i < LIGHT_SOURCES; ++i) {
        lightSources[i] = generateLightSource();
        std::cout << lightSources[i].start_angle << ":" << lightSources[i].end_angle << " -> " << lightSources[i].
                intensity << std::endl;
    }
    MSTree irradianceTree = setupIrradianceTree(lightSources);

    // TOOD only works for single light for now
    float analyticalColor = calculateAnalyticalIrradiance(lightSources);
    std::cout << "analyticalColor: " << analyticalColor << std::endl;

    const Polar wi{1.f, randDistr(randEng) * M_PI_F};

    float brdfReferenceColor = sampling::sample_brdf(REFERENCE_SAMPLES, diffuse, lightSources, wi);
    std::cout << "BRDF reference: " << brdfReferenceColor << std::endl;
    float brdfBenchmarkColor = sampling::sample_brdf(BENCHMARK_SAMPLES, diffuse, lightSources, wi);
    std::cout << "BRDF benchmark: " << brdfBenchmarkColor << std::endl;

    float msTreeReferenceColor = sampling::sample_mstree(REFERENCE_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    std::cout << "MSTree reference: " << msTreeReferenceColor << std::endl;
    float msTreeBenchmarkColor = sampling::sample_mstree(BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    std::cout << "MSTree benchmark: " << msTreeBenchmarkColor << std::endl;

    return 0;
}

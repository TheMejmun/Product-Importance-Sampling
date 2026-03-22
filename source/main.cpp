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
#include "utils.h"
#include "mts/microfacet.h"
#include "renderers/mis.h"
#include "renderers/mstree_sampling.h"

constexpr float M_PI_F = M_PI;
constexpr uint32_t LIGHT_SOURCES = 4;
constexpr float LIGHT_MAX_INTENSITY = 100.0f;
constexpr uint32_t MS_TREE_SAMPLES = 1024;

constexpr uint32_t REFERENCE_SAMPLES = 1048576; // 2 ^ 20
constexpr uint32_t BENCHMARK_SAMPLES = 64;

constexpr uint32_t MICROFACET_TEST_SAMPLE_COUNT = 1000000;

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

struct Sample {
    Vec3f normal;
    Vec3f wo;
    float pdf;
};

Sample sampleMicrofacet3D(const mts::MicrofacetDistribution &distr, const Vec3f &wi) {
    const Point2f sample = {randDistr(randEng), randDistr(randEng)};
    const auto [normal, pdf] = distr.sample(wi, sample);
    const Vec3f wo = utils::reflect(wi, normal);
    return {normal, wo, pdf};
}

void testMicrofacet3D() {
    mts::MicrofacetDistribution test{0.1f};
    const auto wi = utils::normalize(Vec3f{1.0f, 0.0f, 1.0f});
    printf("wi=[%f, %f, %f]\n", wi.x, wi.y, wi.z);

    std::vector<Sample> samples(MICROFACET_TEST_SAMPLE_COUNT);
    for (int i = 0; i < MICROFACET_TEST_SAMPLE_COUNT; ++i) {
        samples[i] = sampleMicrofacet3D(test, wi);
    }

    float avgPdf = 0.f;
    Vec3f avgNormal = {0.f, 0.f, 0.f};
    Vec3f avgWo = {0.f, 0.f, 0.f};
    for (const auto &[normal,wo,pdf]: samples) {
        avgPdf += pdf;
        avgNormal = avgNormal + normal;
        avgWo = avgWo + wo;
    }
    avgPdf = avgPdf / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);
    avgNormal = avgNormal / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);
    avgWo = avgWo / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);

    float varPdf = 0.f;
    Vec3f varNormal = {0.f, 0.f, 0.f};
    Vec3f varWo = {0.f, 0.f, 0.f};
    for (const auto &[normal,wo,pdf]: samples) {
        varPdf += (pdf - avgPdf) * (pdf - avgPdf);
        varNormal = varNormal + (normal - avgNormal) * (normal - avgNormal);
        varWo = varWo + (wo - avgWo) * (wo - avgWo);
    }
    varPdf = varPdf / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);
    varNormal = varNormal / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);
    varWo = varWo / static_cast<float>(MICROFACET_TEST_SAMPLE_COUNT);

    printf("avgNormal=[%f, %f, %f], varNormal=[%f, %f, %f]\n", avgNormal.x, avgNormal.y, avgNormal.z, varNormal.x,
           varNormal.y, varNormal.z);
    printf("avgPdf=%f, varPdf=%f\n", avgPdf, varPdf);
    printf("avgWo=[%f, %f, %f], varWo=[%f, %f, %f]\n", avgWo.x, avgWo.y, avgWo.z, varWo.x, varWo.y, varWo.z);
}

int main() {
    testMicrofacet3D();
    return 0;

    DiffuseBRDF diffuse{};
    std::vector<LightSource> lightSources(LIGHT_SOURCES);
    for (uint32_t i = 0; i < LIGHT_SOURCES; ++i) {
        lightSources[i] = generateLightSource();
        std::cout << lightSources[i].start_angle << ":" << lightSources[i].end_angle << " -> " << lightSources[i].
                intensity << std::endl;
    }
    MSTree irradianceTree = setupIrradianceTree(lightSources);

    const Polar wi{1.f, randDistr(randEng) * M_PI_F};

    printf("Reference:\n");
    // TOOD only works for diffuse brdf
    float analytical = calculateAnalyticalIrradiance(lightSources);
    printf("\tAnalytical: %f\n", analytical);
    float brdfReference = sampling::sample_brdf(REFERENCE_SAMPLES, diffuse, lightSources, wi);
    printf("\tBRDF: %f\n", brdfReference);
    float mstReference = sampling::sample_mstree(REFERENCE_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tMST Sampling: %f\n", mstReference);
    float misReference = sampling::mis(REFERENCE_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tMIS: %f\n", misReference);

    printf("Equal Samples %d:\n", BENCHMARK_SAMPLES);
    float brdfBenchmarkES = sampling::sample_brdf(BENCHMARK_SAMPLES, diffuse, lightSources, wi);
    printf("\tBRDF: %f\n", brdfBenchmarkES);
    float mstBenchmarkES = sampling::sample_mstree(BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tMST Sampling: %f\n", mstBenchmarkES);
    float misBenchmarkES = sampling::mis(BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tMIS: %f\n", misBenchmarkES);

    return 0;
}

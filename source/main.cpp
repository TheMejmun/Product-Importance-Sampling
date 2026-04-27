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
#include "renderers/direct_light_sampling.h"

namespace {
    constexpr uint32_t dpow(uint32_t base, uint32_t exp) {
        uint32_t result = 1;
        for (uint32_t i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }
}

constexpr double M_PI_F = M_PI;
constexpr uint32_t LIGHT_SOURCES = 4;
constexpr double LIGHT_MAX_INTENSITY = 100.0;
constexpr uint32_t MS_TREE_SAMPLES = dpow(2, 10);

constexpr uint32_t REFERENCE_SAMPLES = dpow(2, 25);
constexpr uint32_t ES_BENCHMARK_SAMPLES = dpow(2, 6);
constexpr double ET_BENCHMARK_SECONDS = 0.001;

constexpr uint32_t MICROFACET_TEST_SAMPLE_COUNT = dpow(2, 20);
constexpr uint32_t PERTURBED_TEST_SAMPLE_COUNT = dpow(2, 20);
constexpr uint32_t REFLECTED_PDF_TEST_SAMPLE_COUNT = dpow(2, 20);
constexpr uint32_t MANY_PDF_TEST_SAMPLE_COUNT = dpow(2, 4);

void print_constants() {
    std::cout << "LIGHT_SOURCES                   " << LIGHT_SOURCES << std::endl;
    std::cout << "LIGHT_MAX_INTENSITY             " << LIGHT_MAX_INTENSITY << std::endl;
    std::cout << "MS_TREE_SAMPLES                 " << MS_TREE_SAMPLES << std::endl;
    std::cout << "REFERENCE_SAMPLES               " << REFERENCE_SAMPLES << std::endl;
    std::cout << "ES_BENCHMARK_SAMPLES            " << ES_BENCHMARK_SAMPLES << std::endl;
    std::cout << "ET_BENCHMARK_SECONDS            " << ET_BENCHMARK_SECONDS << std::endl;
    std::cout << "MICROFACET_TEST_SAMPLE_COUNT    " << MICROFACET_TEST_SAMPLE_COUNT << "\n" << std::endl;
    std::cout << "PERTURBED_TEST_SAMPLE_COUNT     " << PERTURBED_TEST_SAMPLE_COUNT << std::endl;
    std::cout << "REFLECTED_PDF_TEST_SAMPLE_COUNT " << REFLECTED_PDF_TEST_SAMPLE_COUNT << std::endl;
    std::cout << "MANY_PDF_TEST_SAMPLE_COUNT      " << MANY_PDF_TEST_SAMPLE_COUNT << std::endl;
}

// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<double> randDistr(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> randIntDistr(0, std::numeric_limits<uint32_t>::max());
}

LightSource generateLightSource() {
    const double startAngle = randDistr(randEng) * M_PI_F;
    const double endAngle = randDistr(randEng) * (M_PI_F - startAngle) + startAngle;
    const double intensity = randDistr(randEng);
    return {startAngle, endAngle, intensity};
}

MSTree setupIrradianceTree(const std::vector<LightSource> &lightSources) {
    MSTree irradianceTree{};

    for (int i = 0; i < MS_TREE_SAMPLES; ++i) {
        double angle = randDistr(randEng) * M_PI_F;

        // Get light sources that can be reached in that direction
        std::vector<LightSource const *> reachableLights;
        for (const auto &light: lightSources) {
            if (light.start_angle <= angle && light.end_angle >= angle) {
                reachableLights.push_back(&light);
            }
        }
        // Randomly take one of them for the intensity
        double intensity = reachableLights.empty()
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
    double pdf;
};

Sample sampleMicrofacet3D(const mts::MicrofacetDistribution &distr, const Vec3f &wi) {
    const Point2f sample = {randDistr(randEng), randDistr(randEng)};
    const Normal3f normal = distr.sample(wi, sample);
    const double pdf = distr.solid_angle_density(wi, normal);
    const Vec3f wo = utils::reflect(wi, normal);
    return {normal, wo, pdf};
}

void testMicrofacet3D() {
    mts::MicrofacetDistribution test{0.1f};
    const auto wi = utils::normalize(Vec3f{1.0, 0.0, 1.0});
    printf("wi=[%f, %f, %f]\n", wi.x, wi.y, wi.z);

    std::vector<Sample> samples(MICROFACET_TEST_SAMPLE_COUNT);
    for (int i = 0; i < MICROFACET_TEST_SAMPLE_COUNT; ++i) {
        samples[i] = sampleMicrofacet3D(test, wi);
    }

    double avgPdf = 0.0;
    Vec3f avgNormal = {0.0, 0.0, 0.0};
    Vec3f avgWo = {0.0, 0.0, 0.0};
    for (const auto &[normal,wo,pdf]: samples) {
        avgPdf += pdf;
        avgNormal = avgNormal + normal;
        avgWo = avgWo + wo;
    }
    avgPdf = avgPdf / static_cast<double>(MICROFACET_TEST_SAMPLE_COUNT);
    avgNormal = avgNormal / MICROFACET_TEST_SAMPLE_COUNT;
    avgWo = avgWo / MICROFACET_TEST_SAMPLE_COUNT;

    double varPdf = 0.0;
    Vec3f varNormal = {0.0, 0.0, 0.0};
    Vec3f varWo = {0.0, 0.0, 0.0};
    for (const auto &[normal,wo,pdf]: samples) {
        varPdf += (pdf - avgPdf) * (pdf - avgPdf);
        varNormal = varNormal + (normal - avgNormal) * (normal - avgNormal);
        varWo = varWo + (wo - avgWo) * (wo - avgWo);
    }
    varPdf = varPdf / static_cast<double>(MICROFACET_TEST_SAMPLE_COUNT);
    varNormal = varNormal / MICROFACET_TEST_SAMPLE_COUNT;
    varWo = varWo / MICROFACET_TEST_SAMPLE_COUNT;

    printf("avgNormal=[%f, %f, %f], varNormal=[%f, %f, %f]\n", avgNormal.x, avgNormal.y, avgNormal.z, varNormal.x,
           varNormal.y, varNormal.z);
    printf("avgPdf=%f, varPdf=%f\n", avgPdf, varPdf);
    printf("avgWo=[%f, %f, %f], varWo=[%f, %f, %f]\n", avgWo.x, avgWo.y, avgWo.z, varWo.x, varWo.y, varWo.z);
}

void testAzimuthPerturbation() {
    mts::MicrofacetDistribution distr{0.1f};
    std::vector<double> samples(PERTURBED_TEST_SAMPLE_COUNT);
    const Vec3f wi = utils::hemisphereSample();
    const Vec3f m = utils::hemisphereSample();
    const Spherical mSpherical = utils::toSpherical(m);
    const double pdf = distr.solid_angle_density(wi, m);
    if (pdf <= 0.0) {
        testAzimuthPerturbation();
        return;
    }
    printf("Testing effects of azimuth perturbation on pdf\n");
    for (uint32_t i = 0; i < PERTURBED_TEST_SAMPLE_COUNT; ++i) {
        const Spherical perturbedMSpherical{1, mSpherical.theta, 2 * M_PI_F * randDistr(randEng)};
        const Vec3f perturbedM = utils::toVec(perturbedMSpherical);
        samples[i] = distr.solid_angle_density(wi, perturbedM);
    }
    const double mse = utils::mse(pdf, samples);
    printf("MSE: %f\n", mse);

    // Result: sometimes slightly changes pdf, but probably because of numerics only
}

void testSolidAngleDensity() {
    printf("Testing solid angle density accuracy\n");
    mts::MicrofacetDistribution distr{0.1f};
    const double uniform_hemisphere_pdf = 1.0 / (2.0 * M_PI);

    for (double cos_i: {0.9f, 0.7f, 0.5f, 0.3f, 0.1f}) {
        const double sin_i = std::sqrt(1.f - cos_i * cos_i);
        const Vec3f wi = {sin_i, 0.f, cos_i};

        double sum = 0.0;
        for (uint32_t i = 0; i < REFLECTED_PDF_TEST_SAMPLE_COUNT; ++i) {
            const Vec3f m = utils::hemisphereSample();
            // if (utils::cosTheta(utils::reflect(wi, m)) < 0.0) {
            //     --i;
            //     continue;
            // }
            const double pdf = distr.solid_angle_density(wi, m);
            sum += pdf / uniform_hemisphere_pdf;
        }
        printf("cos_i=%.1f: integral = %f\n", cos_i,
               sum / static_cast<double>(REFLECTED_PDF_TEST_SAMPLE_COUNT));
    }
}

void testPDF() {
    printf("Testing PDF accuracy\n");
    mts::MicrofacetDistribution distr{0.1f};
    const double uniform_hemisphere_pdf = 1.0 / (2.0 * M_PI);

    for (double cos_i: {0.9f, 0.7f, 0.5f, 0.3f, 0.1f}) {
        const double sin_i = std::sqrt(1.f - cos_i * cos_i);
        const Vec3f wi = {sin_i, 0.f, cos_i};

        double sum = 0.0;
        for (uint32_t i = 0; i < REFLECTED_PDF_TEST_SAMPLE_COUNT; ++i) {
            const Vec3f m = distr.sample(wi, {randDistr(randEng), randDistr(randEng)});
            if (utils::cosTheta(utils::reflect(wi, m)) < 0.0) {
                --i;
                continue;
            }
            const double pdf = distr.reflected_pdf(wi, m);
            sum += 1.0 / (pdf / uniform_hemisphere_pdf);
        }
        printf("cos_i=%.1f: integral = %f\n", cos_i,
               sum / static_cast<double>(REFLECTED_PDF_TEST_SAMPLE_COUNT));
    }
}

// TODO change to MSE for error calculation
// TODO calculate MSE/Variance per sample
// TODO convert solid angle density to pdf
// TODO reduce Microfacet Dimension to elevation only
// TODO test brightness against Diffuse BRDF with full dome of light
// TODO test brdf sampling mean against direct light sampling mean with microfacet
int main() {
    print_constants();
    printf("\n");

    testAzimuthPerturbation();
    printf("\n");

    testSolidAngleDensity();
    printf("\n");

    testPDF();
    printf("\n");

    DiffuseBRDF diffuse{};
    std::vector<LightSource> lightSources(LIGHT_SOURCES);
    for (uint32_t i = 0; i < LIGHT_SOURCES; ++i) {
        lightSources[i] = generateLightSource();
        std::cout << lightSources[i].start_angle << ":" << lightSources[i].end_angle << " -> " << lightSources[i].
                intensity << std::endl;
    }
    MSTree irradianceTree = setupIrradianceTree(lightSources);

    const Polar wi{1.0, randDistr(randEng) * M_PI_F};

    double brdfReference = sampling::sample_brdf(REFERENCE_SAMPLES, diffuse, lightSources, wi);
    printf("Reference: %f\n", brdfReference);

    printf("Equal Samples %d:\n", ES_BENCHMARK_SAMPLES);
    double brdfBenchmarkES = sampling::sample_brdf(ES_BENCHMARK_SAMPLES, diffuse, lightSources, wi);
    printf("\tBRDF: %f\n", brdfBenchmarkES);
    double mstBenchmarkES = sampling::sample_light(ES_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tDirect Light: %f\n", mstBenchmarkES);
    double misBenchmarkES = sampling::mis(ES_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, wi);
    printf("\tMIS: %f\n", misBenchmarkES);

    printf("Equal Time %f:\n", ET_BENCHMARK_SECONDS);
    double brdfBenchmarkET = sampling::sample_brdf(ET_BENCHMARK_SECONDS, diffuse, lightSources, wi);
    printf("\tBRDF: %f\n", brdfBenchmarkET);
    double mstBenchmarkET = sampling::sample_light(ET_BENCHMARK_SECONDS, irradianceTree, diffuse, lightSources, wi);
    printf("\tDirect Light: %f\n", mstBenchmarkET);
    double misBenchmarkET = sampling::mis(ET_BENCHMARK_SECONDS, irradianceTree, diffuse, lightSources, wi);
    printf("\tMIS: %f\n", misBenchmarkET);
    return 0;
}

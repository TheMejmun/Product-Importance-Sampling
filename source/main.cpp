//
// Created by Saman on 29.12.25.
//

#include <cstdio>
#include <iostream>
#include <ostream>

#include "brdf/brdf.h"
#include "renderers/brdf_sampler.h"
#include "brdf/diffuse_brdf.h"
#include "light_source.h"
#include "multistage_tree.h"
#include "utils.h"
#include "brdf/microfacet_brdf.h"
#include "mts/microfacet.h"
#include "renderers/mis_sampler.h"
#include "renderers/direct_light_sampler.h"
#include "renderers/pis_sampler.h"

#define UNIFORM_LIGHT
#define RANDOM_WI
// #define CONSTANT_WI

namespace {
    constexpr uint32_t dpow(uint32_t base, uint32_t exp) {
        uint32_t result = 1;
        for (uint32_t i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }
}

constexpr uint32_t LIGHT_SOURCES = 4;
constexpr double LIGHT_MAX_INTENSITY = 100.0;
constexpr uint32_t MS_TREE_SAMPLES = dpow(2, 20);

constexpr uint32_t REFERENCE_SAMPLES = dpow(2, 10);
constexpr uint32_t ES_BENCHMARK_SAMPLES = dpow(2, 6);
constexpr uint32_t MSE_BENCHMARK_SAMPLES = dpow(2, 15);
constexpr double ET_BENCHMARK_SECONDS = 0.0001;

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
    std::cout << "MSE_BENCHMARK_SAMPLES           " << MSE_BENCHMARK_SAMPLES << std::endl;
    std::cout << "ET_BENCHMARK_SECONDS            " << ET_BENCHMARK_SECONDS << std::endl;
    std::cout << "MICROFACET_TEST_SAMPLE_COUNT    " << MICROFACET_TEST_SAMPLE_COUNT << std::endl;
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
    const double startAngle = randDistr(randEng) * M_PI;
    const double endAngle = randDistr(randEng) * (M_PI - startAngle) + startAngle;
    const double intensity = randDistr(randEng);
    return {startAngle, endAngle, intensity};
}

MSTree setupIrradianceTree(const std::vector<LightSource> &lightSources) {
    MSTree irradianceTree{};

    for (int i = 0; i < MS_TREE_SAMPLES; ++i) {
        double angle = randDistr(randEng) * M_PI;

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
    printf("Testing effects of azimuth perturbation on pdf\n");
    mts::MicrofacetDistribution distr{0.1f};
    std::vector<double> samples_sa(PERTURBED_TEST_SAMPLE_COUNT);
    std::vector<double> samples_pdf(PERTURBED_TEST_SAMPLE_COUNT);
    const Vec3f wi = utils::hemisphereSample();
    const Vec3f wo = utils::hemisphereSample();
    const Vec3f m = utils::normalize(wi + wo);
    const Spherical mSpherical = utils::toSpherical(m);
    const double sa = distr.solid_angle_density(wi, m);
    const double pdf = distr.reflected_pdf(wi, m);

    for (uint32_t i = 0; i < PERTURBED_TEST_SAMPLE_COUNT; ++i) {
        const Spherical perturbedMSpherical{1, mSpherical.theta, 2 * M_PI * randDistr(randEng)};
        const Vec3f perturbedM = utils::toVec(perturbedMSpherical);
        const double sad = distr.solid_angle_density(wi, perturbedM);
        const double pdf = distr.reflected_pdf(wi, perturbedM);
        if (sad <= 0.0 || pdf <= 0.0) {
            --i;
            continue;
        }
        samples_sa[i] = sad;
        samples_pdf[i] = pdf;
    }
    const double mse_sa = utils::mse(sa, samples_sa);
    printf("\tMSE solid angle: %f\n", mse_sa);
    const double mse_pdf = utils::mse(pdf, samples_pdf);
    printf("\tMSE pdf: %f\n", mse_pdf);

    // Result: sometimes slightly changes pdf, but probably because of numerics only
}

void testPDF() {
    printf("Testing PDF accuracy\n");
    mts::MicrofacetDistribution distr{0.1f};
    const Vec3f wi = utils::hemisphereSample();
    const double uniform_hemisphere_pdf = 1.0 / (2.0 * M_PI);

    double sum = 0.0;
    for (uint32_t i = 0; i < REFLECTED_PDF_TEST_SAMPLE_COUNT; ++i) {
        const Vec3f wo = utils::hemisphereSample();
        const Vec3f m = utils::normalize(wi + wo);
        const double pdf = distr.reflected_pdf(wi, m);
        sum += pdf / uniform_hemisphere_pdf;
    }
    printf("\tintegral wo uniform sampled: %f\n",
           sum / static_cast<double>(REFLECTED_PDF_TEST_SAMPLE_COUNT));

    sum = 0.0;
    for (uint32_t i = 0; i < REFLECTED_PDF_TEST_SAMPLE_COUNT; ++i) {
        const Vec3f m = distr.sample(wi, {randDistr(randEng), randDistr(randEng)});
        const double pdf = distr.reflected_pdf(wi, m);
        sum += pdf;
    }
    printf("\tintegral m distr sampled: %f\n",
           sum / static_cast<double>(REFLECTED_PDF_TEST_SAMPLE_COUNT));
}

// TODO test brightness against Diffuse BRDF with full dome of light
// TODO test brdf sampling mean against direct light sampling mean with microfacet
int main() {
    print_constants();
    printf("\n");

    testAzimuthPerturbation();
    printf("\n");

    testPDF();
    printf("\n");

    DiffuseBRDF diffuse{};
    MicrofacetBRDF microfacet{};

#ifdef UNIFORM_LIGHT
    std::vector<LightSource> lightSources{};
    lightSources.emplace_back(0.0,M_PI, 10.0);
#else
    std::vector<LightSource> lightSources(LIGHT_SOURCES);
    for (uint32_t i = 0; i < LIGHT_SOURCES; ++i) {
        lightSources[i] = generateLightSource();
        std::cout << lightSources[i].start_angle << ":" << lightSources[i].end_angle << " -> " << lightSources[i].
                intensity << std::endl;
    }
#endif

    MSTree irradianceTree = setupIrradianceTree(lightSources);

#ifdef RANDOM_WI
    const Polar wi{1.0, randDistr(randEng) * M_PI};
    const Vec3f wi3 = utils::hemisphereSample();
#else
    const Polar wi{1.0, M_PI / 4.0};
    const Vec3f wi3 = utils::normalize(Vec3f{1.0, 0.0, 1.0});
#endif

#ifdef CONSTANT_WI
    const Polar *p_wi = &wi;
    const Vec3f *p_wi3 = &wi3;
#else
    const Polar *p_wi = nullptr;
    const Vec3f *p_wi3 = nullptr;
#endif


    printf("wi: %s\n", wi.toString().c_str());
    printf("wi3: %s\n", wi3.toString().c_str());

    const BRDFSampler brdf_sampler{};
    const DirectLightSampler direct_light_sampler{};
    const MISSampler mis_sampler{};
    const PISSampler pis_sampler{};

    double brdfReference =
            brdf_sampler.equal_samples(REFERENCE_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("Reference: %f\n", brdfReference);
    double microfacetReference =
            brdf_sampler.equal_samples(REFERENCE_SAMPLES, irradianceTree, microfacet, lightSources, p_wi);
    printf("Reference Microfacet: %f\n", microfacetReference);

    double brdfReference3D =
            brdf_sampler.equal_samples3D(REFERENCE_SAMPLES, diffuse, p_wi3);
    printf("Reference 3D: %f\n", brdfReference3D);
    double microfacetReference3D =
            brdf_sampler.equal_samples3D(REFERENCE_SAMPLES, microfacet, p_wi3);
    printf("Reference 3D Microfacet: %f\n", microfacetReference3D);

    printf("MSE (%d samples):\n", MSE_BENCHMARK_SAMPLES);
    double brdfBenchmarkMSE =
            brdf_sampler.mse(brdfReference, MSE_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tBRDF: %f\n", brdfBenchmarkMSE);
    double mstBenchmarkMSE =
            direct_light_sampler.mse(brdfReference, MSE_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tDirect Light: %f\n", mstBenchmarkMSE);
    double misBenchmarkMSE =
            mis_sampler.mse(brdfReference, MSE_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tMIS: %f\n", misBenchmarkMSE);

    printf("Equal Samples (%d samples):\n", ES_BENCHMARK_SAMPLES);
    double brdfBenchmarkES =
            brdf_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tBRDF: %f\n", brdfBenchmarkES);
    double mstBenchmarkES =
            direct_light_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tDirect Light: %f\n", mstBenchmarkES);
    double misBenchmarkES =
            mis_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tMIS: %f\n", misBenchmarkES);

    printf("Equal Time (%f s):\n", ET_BENCHMARK_SECONDS);
    double brdfBenchmarkET =
            brdf_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tBRDF: %f\n", brdfBenchmarkET);
    double mstBenchmarkET =
            direct_light_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tDirect Light: %f\n", mstBenchmarkET);
    double misBenchmarkET =
            mis_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, diffuse, lightSources, p_wi);
    printf("\tMIS: %f\n", misBenchmarkET);
    return 0;
}

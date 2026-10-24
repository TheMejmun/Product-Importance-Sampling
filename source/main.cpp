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

// #define RANDOM_WI

// Combine any of these light sources to create different test scenarios
// #define UNIFORM_LIGHT // uniform light source
#define ZENITH_LIGHT // strong light at peak of hemisphere, 2% of hemisphere area
#define FRONT_LIGHT // strong light from behind wi, 2% area
#define BACK_LIGHT // strong light from opposite wi (reflect wi around up normal), 2% area

#define MICROFACET_BRDF

namespace {
    constexpr uint32_t dpow(uint32_t base, uint32_t exp) {
        uint32_t result = 1;
        for (uint32_t i = 0; i < exp; ++i) {
            result *= base;
        }
        return result;
    }

    // For the `constant` macro
    template<typename T>
    constexpr T print_and_return(const char *name, const T &v) {
        std::cout << name << ":";
        for (uint32_t i = 1; i < 32; ++i) {
            if (strlen(name) < i) {
                std::cout << " ";
            }
        }
        std::cout << v << std::endl;
        return v;
    }
}

#define constant(name, value) const auto name = print_and_return(#name, value)

constant(LIGHT_MAX_INTENSITY, 1000.0);
constant(MS_TREE_SAMPLES, dpow(2, 20));
constant(REFERENCE_SAMPLES, dpow(2, 20));
constant(ES_BENCHMARK_SAMPLES, dpow(2, 10));
constant(MSE_BENCHMARK_SAMPLES, dpow(2, 20));
#ifdef NDEBUG
constant(ET_BENCHMARK_SECONDS, 0.001);
#else
constant(ET_BENCHMARK_SECONDS, 0.01);
#endif
constant(MICROFACET_TEST_SAMPLE_COUNT, dpow(2, 20));
constant(MSE_COUNT, dpow(2, 10));
#define _mse(func) utils::mse<double>(reference, MSE_COUNT, [&]{calculateTestSetup(wi, lightSources, irradianceTree, reference, brdf); return func;})

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
    const double intensity = randDistr(randEng) * LIGHT_MAX_INTENSITY;
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

// TODO test brightness against Diffuse BRDF with full dome of light
// TODO test brdf sampling mean against direct light sampling mean with microfacet

struct Wi {
    Polar polar;
    Vec3f vec3;
};

static Wi calculateWi() {
#ifdef RANDOM_WI
    const Polar wi{1.0, randDistr(randEng) * M_PI};
    const Vec3f wi3 = utils::hemisphereSample();
#else
    const Polar wi{1.0, M_PI / 4.0};
    const Vec3f wi3 = utils::normalize(Vec3f{1.0, 0.0, 1.0});
#endif

    // printf("wi: %s\n", wi.toString().c_str());
    // printf("wi3: %s\n", wi3.toString().c_str());

    return {wi, wi3};
}

void setupLights(std::vector<LightSource> &lightSources, const Polar wi) {
    lightSources.clear();

    // Should be fine for light sources to go below the horizontal plane.
    // In the case of FRONT_LIGHT or BACK_LIGHT, when wi.phi is close to 0.0 or M_PI
#ifdef UNIFORM_LIGHT
    lightSources.emplace_back(0.0, M_PI, LIGHT_MAX_INTENSITY / 100.0); // 100% hemisphere coverage
#endif
#ifdef ZENITH_LIGHT
    lightSources.emplace_back(M_PI * 0.49, M_PI * 0.51, LIGHT_MAX_INTENSITY); // 2% hemisphere coverage
#endif
#ifdef FRONT_LIGHT
    double directionFront = wi.phi;
    lightSources.emplace_back(directionFront - M_PI * 0.01, directionFront + M_PI * 0.01, LIGHT_MAX_INTENSITY);
    // 2% hemisphere coverage
#endif
#ifdef BACK_LIGHT
    double directionBack = utils::reflect(wi, Polar(1.0, M_PI / 2)).phi; // reflect wi around vertical axis
    lightSources.emplace_back(directionBack - M_PI * 0.01, directionBack + M_PI * 0.01, LIGHT_MAX_INTENSITY);
    // 2% hemisphere coverage
#endif
}

struct TestSetup {
    uint32_t samples;
    double time;

    Polar wi;
    BRDF &brdf;
    double reference;

    MSTree &irradianceTree;
    std::vector<LightSource> &lightSources;
};

void calculateTestSetup(
    Polar &wi,
    std::vector<LightSource> &lightSources,
    MSTree &irradianceTree,
    double &reference,
    BRDF &brdf
) {
    // printf("\nGenerating wi\n");
    wi = calculateWi().polar;

    // printf("\nSetting up light sources\n");
    setupLights(lightSources, wi);

    // printf("\nSetting up irradiance tree\n");
    irradianceTree = setupIrradianceTree(lightSources);

    // printf("\nCalculating reference\n");
    const BRDFSampler brdf_sampler{};
    reference = brdf_sampler.equal_samples(REFERENCE_SAMPLES, irradianceTree, brdf, lightSources, wi);

    // printf("\nCalculating uniform sample mapping\n");
    brdf.calculateMapping(irradianceTree, wi);
}

int main() {
    Polar wi{};
    std::vector<LightSource> lightSources{};
    MSTree irradianceTree{};
    double reference;

    DiffuseBRDF diffuse{};
    MicrofacetBRDF microfacet{};
#ifdef MICROFACET_BRDF
    BRDF &brdf = microfacet;
#else
    BRDF &brdf = diffuse;
#endif

    const BRDFSampler brdf_sampler{};
    const DirectLightSampler light_sampler{};
    const MISSampler mis_sampler{};
    const PISSampler pis_sampler{};

    calculateTestSetup(wi, lightSources, irradianceTree, reference, brdf);

    printf("\n");

    printf("Per-Sample MSE (%d samples, same wi):\n", MSE_BENCHMARK_SAMPLES);
    double brdf_mse = brdf_sampler.mse(reference, MSE_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi);
    printf("\tBRDF Sampling: %f\n", brdf_mse);
    double mst_mse = light_sampler.mse(reference, MSE_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi);
    printf("\tLight Sampling: %f\n", mst_mse);
    double mis_mse = mis_sampler.mse(reference, MSE_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi);
    printf("\tMIS: %f\n", mis_mse);
    double pis_mse = pis_sampler.mse(reference, MSE_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi);
    printf("\tPIS: %f\n", pis_mse);

    printf("Equal Samples (MSE of %d renders, new wi and %d samples each):\n", MSE_COUNT, ES_BENCHMARK_SAMPLES);
    double brdf_es = _mse(brdf_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi));
    printf("\tBRDF Sampling: %f\n", brdf_es);
    double mst_es = _mse(light_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi));
    printf("\tLight Sampling: %f\n", mst_es);
    double mis_es = _mse(mis_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi));
    printf("\tMIS: %f\n", mis_es);
    double pis_es = _mse(pis_sampler.equal_samples(ES_BENCHMARK_SAMPLES, irradianceTree, brdf, lightSources, wi));
    printf("\tPIS: %f\n", pis_es);

    printf("Equal Time (MSE of %d renders, new wi and %fs each)\n", MSE_COUNT, ET_BENCHMARK_SECONDS);
    double brdf_et = _mse(brdf_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, brdf, lightSources, wi));
    printf("\tBRDF Sampling: %f\n", brdf_et);
    double mst_et = _mse(light_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, brdf, lightSources, wi));
    printf("\tLight Sampling: %f\n", mst_et);
    double mis_et = _mse(mis_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, brdf, lightSources, wi));
    printf("\tMIS: %f\n", mis_et);
    double pis_et = _mse(pis_sampler.equal_time(ET_BENCHMARK_SECONDS, irradianceTree, brdf, lightSources, wi));
    printf("\tPIS: %f\n", pis_et);

    return 0;
}

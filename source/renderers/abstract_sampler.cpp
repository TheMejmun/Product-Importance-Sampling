//
// Created by Saman on 28.04.26.
//

#include "renderers/abstract_sampler.h"

#include <chrono>

// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution randDistr(0.0, 1.0);
}

double AbstractSampler::equal_samples(std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                                      const std::vector<LightSource> &lightSources, const Polar &wi) const {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += sample(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<double>(iterations);


    return color;
}

double AbstractSampler::mse(double reference, std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                            const std::vector<LightSource> &lightSources, const Polar &wi) const {
    double mse = 0.0;
    for (int i = 0; i < iterations; ++i) {
        const double color = sample(msTree, brdf, lightSources, wi);
        mse += (color - reference) * (color - reference);
    }
    mse /= static_cast<double>(iterations);
    return mse;
}

double AbstractSampler::equal_time(double seconds, const MSTree &msTree, const BRDF &brdf,
                                   const std::vector<LightSource> &lightSources, const Polar &wi) const {
    double color = 0.0;
    int iterations = 0;
    const auto start = std::chrono::steady_clock::now();
    for (;;) {
        const double res = sample(msTree, brdf, lightSources, wi);
        const std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start;
        if (duration.count() > seconds) {
            break;
        }
        color += res;
        ++iterations;
    }
    printf("\t%d samples in %f seconds\t", iterations, seconds);
    color /= static_cast<double>(iterations);
    return color;
}

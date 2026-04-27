//
// Created by Saman on 07.02.26.
//

#include "renderers/direct_light_sampling.h"

// TODO cosTheta wi
double sampling::sample_light(
    std::uint32_t iterations,
    const MSTree &msTree,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += sample_light(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<double>(iterations);
    return color;
}

double sampling::sample_light(double seconds, const MSTree &msTree, const BRDF &brdf,
                              const std::vector<LightSource> &lightSources, const Polar &wi) {
    double color = 0.0;
    int iterations = 0;
    time_t start;
    time(&start);
    for (;;) {
        const double res = sample_light(msTree, brdf, lightSources, wi);
        time_t now;
        time(&now);
        const double duration = difftime(now, start);
        if (duration > seconds) {
            break;
        }
        color += res;
        ++iterations;
    }
    printf("\t%d samples in %f seconds\t", iterations, seconds);
    color /= static_cast<double>(iterations);
    return color;
}


// TODO cosTheta wo
double sampling::sample_light(
    const MSTree &msTree,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    const MSTSample sample = msTree.sample();
    const double incomingLight = intersect_lights(lightSources, sample.wo);
    return incomingLight * brdf.eval(wi, sample.wo) / sample.pdf;
}

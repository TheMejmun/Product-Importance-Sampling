//
// Created by Saman on 05.02.26.
//

#include "../../include/renderers/brdf_sampling.h"

#include "light_source.h"

// TODO cosTheta wi
double sampling::sample_brdf(
    std::uint32_t iterations,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += sample_brdf(brdf, lightSources, wi);
    }
    color /= static_cast<double>(iterations);
    return color;
}

double sampling::sample_brdf(double seconds, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                             const Polar &wi) {
    double color = 0.0;
    int iterations = 0;
    time_t start;
    time(&start);
    for (;;) {
        const double res = sample_brdf(brdf, lightSources, wi);
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
double sampling::sample_brdf(const BRDF &brdf, const std::vector<LightSource> &lightSources, const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const double incomingLight = intersect_lights(lightSources, wo);
    const double pdf = brdf.pdf(wi, wo);
    if (pdf <= 0.0) {
        // printf("pdf <= 0.0\n");
        return sample_brdf(brdf, lightSources, wi);
    }
    const double result = incomingLight * brdf.eval(wi, wo) / brdf.pdf(wi, wo);
    // printf("%s color: %f\n", brdf.name(), result);
    return result;
}

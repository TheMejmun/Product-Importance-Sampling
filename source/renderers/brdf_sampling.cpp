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

// TODO cosTheta wo
double sampling::sample_brdf(const BRDF &brdf, const std::vector<LightSource> &lightSources, const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const double incomingLight = intersect_lights(lightSources, wo);
    return incomingLight * brdf.eval(wi, wo) / brdf.pdf(wi, wo);
}

//
// Created by Saman on 05.02.26.
//

#include "../../include/renderers/brdf_sampling.h"

#include "light_source.h"

float sampling::sample_brdf(
    std::uint32_t iterations,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    float color = 0.f;
    for (int i = 0; i < iterations; ++i) {
        color += sample_brdf(brdf, lightSources, wi);
    }
    color /= static_cast<float>(iterations);
    return color;
}

float sampling::sample_brdf(const BRDF &brdf, const std::vector<LightSource> &lightSources, const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const float incomingLight = intersect_lights(lightSources, wo);
    return incomingLight * brdf.eval(wi, wo) / brdf.pdf(wi, wo);
}

//
// Created by Saman on 07.02.26.
//

#include "renderers/mstree_sampling.h"

// TODO cosTheta wi
float sampling::sample_mstree(
    std::uint32_t iterations,
    const MSTree &msTree,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    float color = 0.f;
    for (int i = 0; i < iterations; ++i) {
        color += sample_mstree(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<float>(iterations);
    return color;
}

// TODO cosTheta wo
float sampling::sample_mstree(
    const MSTree &msTree,
    const BRDF &brdf,
    const std::vector<LightSource> &lightSources,
    const Polar &wi
) {
    const MSTSample sample = msTree.sample();
    const float incomingLight = intersect_lights(lightSources, sample.wo);
    return incomingLight * brdf.eval(wi, sample.wo) / sample.pdf;
}

//
// Created by Saman on 07.02.26.
//

#include "renderers/direct_light_sampler.h"

#include "utils.h"

double DirectLightSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                  const Polar &wi) const {
    const MSTSample sample = msTree.sample();
    const double incomingLight = sampling::intersect_lights(lightSources, sample.wo);
    return incomingLight * brdf.eval(wi, sample.wo) * utils::cosTheta(sample.wo) / sample.pdf;
}

//
// Created by Saman on 07.02.26.
//

#include "renderers/direct_light_sampler.h"

#include "utils.h"

double DirectLightSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                  const Polar &wo) const {
    const MSTSample sample = msTree.sample();
    const double incomingLight = sampling::intersect_lights(lightSources, sample.wi);
    return incomingLight * brdf.eval(wo, sample.wi) * utils::cosTheta(sample.wi) / sample.pdf;
}

//
// Created by Saman on 05.02.26.
//

#include "../../include/renderers/brdf_sampler.h"

#include "light_source.h"

// TODO cosTheta wi, wo
double BRDFSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                           const Polar &wi) const {
    const Polar wo = brdf.sample(wi);
    const double incomingLight = sampling::intersect_lights(lightSources, wo);
    const double pdf = brdf.pdf(wi, wo);
    if (pdf <= 0.0) {
        return sample(msTree, brdf, lightSources, wi);
    }
    const double result = incomingLight * brdf.eval(wi, wo) / pdf;
    return result;
}

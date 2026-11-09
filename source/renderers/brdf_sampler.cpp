//
// Created by Saman on 05.02.26.
//

#include "../../include/renderers/brdf_sampler.h"

#include "light_source.h"
#include "utils.h"

// TODO cosTheta wo, wi
double BRDFSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                           const Polar &wo) const {
    const Polar wi = brdf.sample(wo);
    guard_positive(utils::cosTheta(wi));
    const double incomingLight = sampling::intersect_lights(lightSources, wi);
    const double pdf = brdf.pdf(wo, wi);
    guard_positive(pdf);
    const double result = incomingLight * brdf.eval(wo, wi) * utils::cosTheta(wi) / pdf;
    return result;
}

double BRDFSampler::sample3D(const BRDF &brdf, const Vec3f &wo) const {
    const Vec3f wi = brdf.sample(wo);
    guard_positive(utils::cosTheta(wi));
    // TODO
    // Use constant uniform light for all directions.
    const double incomingLight = 10.0;
    const double pdf = brdf.pdf(wo, wi);
    guard_positive(pdf);
    const double result = incomingLight * brdf.eval(wo, wi) * utils::cosTheta(wi) / pdf;
    return result;
}

double BRDFSampler::equal_samples3D(std::uint32_t iterations, const BRDF &brdf, const Vec3f &wo) const {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += sample3D(brdf, wo);
    }
    color /= static_cast<double>(iterations);
    return color;
}

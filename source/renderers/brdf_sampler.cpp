//
// Created by Saman on 05.02.26.
//

#include "../../include/renderers/brdf_sampler.h"

#include "light_source.h"
#include "utils.h"

// TODO cosTheta wi, wo
double BRDFSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                           const Polar &wi) const {
    const Polar wo = brdf.sample(wi);
    guard_positive(utils::cosTheta(wo));
    const double incomingLight = sampling::intersect_lights(lightSources, wo);
    const double pdf = brdf.pdf(wi, wo);
    guard_positive(pdf);
    const double result = incomingLight * brdf.eval(wi, wo) / pdf;
    return result;
}

double BRDFSampler::sample3D(const BRDF &brdf, const Vec3f &wi) const {
    const Vec3f wo = brdf.sample(wi);
    guard_positive(utils::cosTheta(wo));
    // Use constant uniform light for all directions.
    const double incomingLight = 10.0;
    const double pdf = brdf.pdf(wi, wo);
    guard_positive(pdf);
    const double result = incomingLight * brdf.eval(wi, wo) / pdf;
    return result;
}

double BRDFSampler::equal_samples3D(std::uint32_t iterations, const BRDF &brdf, const Vec3f *wi) const {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        if (wi == nullptr) {
            const Vec3f wi = utils::hemisphereSample();
            color += sample3D(brdf, wi);
        } else {
            color += sample3D(brdf, *wi);
        }
    }
    color /= static_cast<double>(iterations);
    return color;
}

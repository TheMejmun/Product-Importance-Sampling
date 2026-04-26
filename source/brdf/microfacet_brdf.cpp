//
// Created by Saman on 29.01.26.
//

#include "brdf/microfacet_brdf.h"
#include <cmath>
#include <cassert>
#include <random>

#include "utils.h"

namespace {
    std::random_device r;
    std::default_random_engine re(r());
    std::uniform_real_distribution<double> hemisphereDist(0.0, M_PI);
    std::uniform_real_distribution<double> uniformDist(0.0, 1.0);
}

Polar truncate(const Vec3f &v) {
    const Spherical s = utils::toSpherical(v);
    return {s.r, s.theta};
}

Vec3f upscale(const Polar &p) {
    const Spherical s{p.r, 0.0, p.phi};
    return utils::toVec(s);
}

double MicrofacetBRDF::eval(const Polar &wi, const Polar &wo) const {
}

double MicrofacetBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    // Equal pdf everywhere, since polar angle is uniformly distributed
    return 1.0 / M_PI;
}

Polar MicrofacetBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const Vec3f wi3 = upscale(wi);
    const Vec3f m3 = mDistribution.sample(wi3, {uniformDist(re), uniformDist(re)});
    const Vec3f wo3 = utils::reflect(wi3, m3);
    return truncate(wo3);
}

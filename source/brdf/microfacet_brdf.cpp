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
    const Spherical s{p.r, p.phi, 0.0};
    return utils::toVec(s);
}

double MicrofacetBRDF::eval(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    // White
    return 1.0 / M_PI;
}

double MicrofacetBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }

    Polar m{1.0, (wi.phi + wo.phi) / 2.0};
    Vec3f wi3 = upscale(wi);
    // Vec3f wo3 = upscale(wo);
    Vec3f m3 = upscale(m);
    // printf("wi [%f, %f, %f]  m [%f, %f, %f]  wo [%f, %f, %f]\n", wi3.x, wi3.y, wi3.z, m3.x, m3.y, m3.z, wo3.x, wo3.y, wo3.z);
    double pdf3 = mDistribution.reflected_pdf(wi3, m3);
    assert(pdf3 < 0.5);
    return pdf3 * 2.0; // Instead of hemsiphere (2pi), normalize to arc (pi)
}

Polar MicrofacetBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const Vec3f wi3 = upscale(wi);
    const Vec3f m3 = mDistribution.sample(wi3, {uniformDist(re), uniformDist(re)});
    const Polar m = truncate(m3);
    const Polar wo = utils::reflect(wi, m);
    const Polar wo2 = truncate(utils::reflect(wi3, m3));
    // printf("wo [%f, %f]  wo2 [%f, %f]\n", wo.r, wo.phi, wo2.r, wo2.phi);

    if (wo.phi > M_PI) {
        return sample(wi);
    }
    return wo;
}

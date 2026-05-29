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

const char *MicrofacetBRDF::name() const {
    return "Microfacet";
}

Polar truncate(const Vec3f &v) {
    const Spherical s = utils::toSpherical(v);
    return {s.r, (M_PI / 2.0) - s.theta};
}

Vec3f upscale(const Polar &p) {
    const Spherical s{p.r, (M_PI / 2.0) - p.phi, 0.0};
    return utils::toVec(s);
}

double MicrofacetBRDF::eval(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    // White
    // Polar m{1.0, (wi.phi + wo.phi) / 2.0};
    // Vec3f m3 = upscale(m);
    // return mDistribution.eval(m3);
    return 1 / M_PI;
}

double MicrofacetBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        printf("Not in hemisphere\n");
        return 0.0;
    }

    Polar m{1.0, (wi.phi + wo.phi) / 2.0};
    Vec3f wi3 = upscale(wi);
    Vec3f m3 = upscale(m);
    double pdf3 = mDistribution.reflected_pdf(wi3, m3);
    // printf("After: wi [%f, %f, %f] m [%f, %f, %f] pdf %f\n", wi3.x, wi3.y, wi3.z, m3.x, m3.y, m3.z, pdf3);
    // printf("pdf: %f, pdf3: %f\n", pdf3 * 2.0, pdf3);
    // assert(pdf3 < 0.5);

    return pdf3 * 2.0; // Instead of hemsiphere (2pi), normalize to arc (pi)
}

Polar MicrofacetBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const Vec3f wi3 = upscale(wi);
    const Vec3f m3 = mDistribution.sample(wi3, {uniformDist(re), uniformDist(re)});
    const Polar m = truncate(m3);
    const Polar wo = utils::reflect(wi, m);
    if (wo.phi > M_PI || wo.phi < 0.0) {
        // printf("wo.phi > M_PI\n");
        return sample(wi);
    }
    // printf("Before: wi [%f, %f, %f] m [%f, %f, %f] pdf %f\n", wi3.x, wi3.y, wi3.z, m3.x, m3.y, m3.z,
    //        mDistribution.reflected_pdf(wi3, m3));

    // printf("sampled: %s\n", wo.toString().c_str());
    return wo;
}

double MicrofacetBRDF::eval(const Vec3f &wi, const Vec3f &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    return 1.0 / M_PI;
}

double MicrofacetBRDF::pdf(const Vec3f &wi, const Vec3f &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    const Vec3f m = utils::normalize(wi + wo);
    return mDistribution.reflected_pdf(wi, m);
}

Vec3f MicrofacetBRDF::sample(const Vec3f &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);
    const Vec3f m = mDistribution.sample(wi, {uniformDist(re), uniformDist(re)});
    const Vec3f wo = utils::reflect(wi, m);
    if (utils::cosTheta(wo) < 0) {
        // printf("wo.phi > M_PI\n");
        return sample(wi);
    }
    return wo;
}

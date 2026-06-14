//
// Created by Saman on 29.01.26.
//

#include "brdf/microfacet_brdf.h"
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

double MicrofacetBRDF::eval(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    return 0.5;
}

double MicrofacetBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        printf("Not in hemisphere\n");
        return 0.0;
    }

    Polar m{1.0, (wi.phi + wo.phi) / 2.0};
    return mDistribution2D.reflected_pdf(utils::toVec(wi), utils::toVec(m));
}

Polar MicrofacetBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const Vec2f m = mDistribution2D.sample(utils::toVec(wi), uniformDist(re));
    return utils::reflect(wi, utils::toPolar(m));
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
    return mDistribution3D.reflected_pdf(wi, m);
}

Vec3f MicrofacetBRDF::sample(const Vec3f &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);
    const Vec3f m = mDistribution3D.sample(wi, {uniformDist(re), uniformDist(re)});
    const Vec3f wo = utils::reflect(wi, m);
    return wo;
}

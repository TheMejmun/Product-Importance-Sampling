//
// Created by Saman on 29.01.26.
//

#include "../../include/brdf/diffuse_brdf.h"
#include <cmath>
#include <cassert>
#include <random>

#include "utils.h"

namespace {
    std::random_device r;
    std::default_random_engine re(r());
    std::uniform_real_distribution<float> hemisphereDist(0.f, M_PI);
}

float DiffuseBRDF::eval(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0f);
    assert(utils::cosTheta(wo) >= 0.0f);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0f;
    }
    // White diffuse
    return 1.0f / M_PI;
}

float DiffuseBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0f);
    assert(utils::cosTheta(wo) >= 0.0f);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0f;
    }
    // Equal pdf everywhere, since polar angle is uniformly distributed
    return 1.0f / M_PI;
}

Polar DiffuseBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0f);

    const float phi = hemisphereDist(re);
    return {1.0f, phi};
}

//
// Created by Saman on 29.01.26.
//

#include "diffuse_brdf.h"
#include <cmath>
#include <cassert>
#include <random>

std::random_device r;
std::default_random_engine re(r());
std::uniform_real_distribution<float> hemisphereDist(0.f, M_PI);

float DiffuseBRDF::eval(const Polar &wi, const Polar &wo) {
    assert(wi.phi >= 0.0f && wi.phi <= (2*M_PI));
    assert(wo.phi >= 0.0f && wo.phi <= (2*M_PI));
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0f;
    }
    // White diffuse
    return 1.0f / M_PI;
}

float DiffuseBRDF::pdf(const Polar &wi, const Polar &wo) {
    assert(wi.phi >= 0.0f && wi.phi <= (2*M_PI));
    assert(wo.phi >= 0.0f && wo.phi <= (2*M_PI));
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0f;
    }
    // Equal pdf everywhere, since polar angle is uniformly distributed
    return 1.0f / M_PI;
}

Polar DiffuseBRDF::sample(const Polar &wi) {
    assert(wi.phi >= 0.0f && wi.phi <= (2*M_PI));

    const float phi = hemisphereDist(re);
    return {1.0f, phi};

    // if (Frame::cosTheta(bRec.params.wi) > 0 && Frame::cosTheta(bRec.params.wo) > 0) {
    //     bRec.value = (eval(bRec.params) / pdf(bRec.params)) * Frame::cosTheta(bRec.params.wo);
    // }
}


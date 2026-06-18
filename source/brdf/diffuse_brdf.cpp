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
    std::uniform_real_distribution hemisphereDist(0.0, M_PI);
    std::uniform_real_distribution uniformDist(0.0, 1.0);
}

const char *DiffuseBRDF::name() const {
    return "Diffuse";
}

double DiffuseBRDF::eval(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    return 0.5;
}

double DiffuseBRDF::pdf(const Polar &wi, const Polar &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    return 1.0 / M_PI;
}

Polar DiffuseBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const double phi = hemisphereDist(re);
    return {1.0, phi};
}

double DiffuseBRDF::pdf(const Polar &wi, const Polar &wo, uint32_t nodeIndex) const {
    throw std::runtime_error("Not implemented");
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    if (wi.phi > M_PI || wo.phi > M_PI) {
        return 0.0;
    }
    const Range &range = mRangeMappings[nodeIndex];
    const double rangeWidth = range.end - range.start;
    if (rangeWidth <= 0.0) { return 0.0; }
    return 1.0 / (rangeWidth * M_PI);
}

Polar DiffuseBRDF::sample(const Polar &wi, uint32_t nodeIndex) const {
    assert(utils::cosTheta(wi) >= 0.0);

    const Range &range = mRangeMappings[nodeIndex];
    const double rangeWidth = range.end - range.start;
    const double uniform = range.start + rangeWidth * uniformDist(re);
    return {1.0, uniform * M_PI};
}

double DiffuseBRDF::eval(const Vec3f &wi, const Vec3f &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    return 1.0 / M_PI;
}

double DiffuseBRDF::pdf(const Vec3f &wi, const Vec3f &wo) const {
    assert(utils::cosTheta(wi) >= 0.0);
    assert(utils::cosTheta(wo) >= 0.0);
    // Equal pdf everywhere, since polar angle is uniformly distributed
    return 1.0 / (2 * M_PI);
}

Vec3f DiffuseBRDF::sample(const Vec3f &wi) const {
    assert(utils::cosTheta(wi) >= 0.0);
    return utils::hemisphereSample();
}

void DiffuseBRDF::calculateMapping(const MSTree &msTree, const Polar &wi) {
    const auto &nodes = msTree.getNodes();
    mRangeMappings = std::vector<Range>(nodes.size());
    for (uint32_t i = 0; i < nodes.size(); ++i) {
        mRangeMappings[i].start = nodes[i].startBoundary / M_PI;
        mRangeMappings[i].end = nodes[i].endBoundary / M_PI;
    }
}

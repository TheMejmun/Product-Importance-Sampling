//
// Created by Saman on 31.12.25.
//

#include "../../include/brdf/microfacet_brdf.h"
#include "utils.h"
#include "coords.h"
#include <cassert>
#include <cmath>
#include <random>
// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/bsdfs/roughconductor.cpp#L257

// Made some assumptions when porting
//   sampleVisible is always true
//   The type of microfacet model is always EGGX
//   There is no intersection location, as we are only analyzing a single point
//   is isotropic -> alphaU and alphaV are the same
//   for normal sampling we only take the x component TODO is this correct?

namespace {
    std::random_device r;
    std::default_random_engine re(r());
    std::uniform_real_distribution<float> uniformDistr(0.f, 1.0f);
}

// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/libcore/util.cpp#L715
float fresnelConductorExact(float cosThetaI, float eta, float k) {
    /* Modified from "Optics" by K.D. Moeller, University Science Books, 1988 */

    float cosThetaI2 = cosThetaI * cosThetaI,
            sinThetaI2 = 1 - cosThetaI2,
            sinThetaI4 = sinThetaI2 * sinThetaI2;

    float temp1 = eta * eta - k * k - sinThetaI2,
            a2pb2 = sqrt(std::max(0.0f, temp1 * temp1 + 4 * k * k * eta * eta)),
            a = sqrt(std::max(0.0f, 0.5f * (a2pb2 + temp1)));

    float term1 = a2pb2 + cosThetaI2,
            term2 = 2 * a * cosThetaI;

    float Rs2 = (term1 - term2) / (term1 + term2);

    float term3 = a2pb2 * cosThetaI2 + sinThetaI4,
            term4 = term2 * sinThetaI2;

    float Rp2 = Rs2 * (term3 - term4) / (term3 + term4);

    return 0.5f * (Rp2 + Rs2);
}

// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/bsdfs/microfacet.h#L573
float sampleMicrofacetDistr11(float thetaI) {
    const float SQRT_PI_INV = 1 / std::sqrt(M_PI);

    float sampleX = uniformDistr(re);
    float sampleY = uniformDistr(re);

    /* Special case (normal incidence) */
    if (thetaI < 1e-4f) {
        float r = sqrt(std::max(0.0f, sampleX / (1 - sampleX)));
        return r;
    }

    /* Precomputations */
    float tanThetaI = std::tan(thetaI);
    float a = 1 / tanThetaI;
    float G1 = 2.0f / (1.0f + sqrt(std::max(0.0f, 1.0f + 1.0f / (a * a))));

    /* Simulate X component */
    float A = 2.0f * sampleX / G1 - 1.0f;
    if (std::abs(A) == 1)
        A -= (A < 0 ? -1.0f : 1.0f) * 1e-4f;
    float tmp = 1.0f / (A * A - 1.0f);
    float B = tanThetaI;
    float D = sqrt(std::max(0.0f, B * B * tmp * tmp - (A * A - B * B) * tmp));
    float slope_x_1 = B * tmp - D;
    float slope_x_2 = B * tmp + D;
    float slopeX = (A < 0.0f || slope_x_2 > 1.0f / tanThetaI) ? slope_x_1 : slope_x_2;

    return slopeX;

    /* Simulate Y component */
    float S;
    if (sampleY > 0.5f) {
        S = 1.0f;
        sampleY = 2.0f * (sampleY - 0.5f);
    } else {
        S = -1.0f;
        sampleY = 2.0f * (0.5f - sampleY);
    }

    /* Improved fit */
    float z =
            (sampleY * (sampleY * (sampleY * (-(float) 0.365728915865723) + (float) 0.790235037209296) -
                        (float) 0.424965825137544) + (float) 0.000152998850436920) /
            (sampleY * (sampleY * (sampleY * (sampleY * (float) 0.169507819808272 - (float) 0.397203533833404) -
                                   (float) 0.232500544458471) + (float) 1) - (float) 0.539825872510702);

    float slopeY = S * z * std::sqrt(1.0f + slopeX * slopeX);

    return slopeY;
}

// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/bsdfs/microfacet.h#L421
Vec2f sampleMicrofacetDistr(const Vec2f &_wi, float alpha) {
    /* Step 1: stretch wi */
    Vec2f wi = utils::normalize(Vec2f(
        alpha * _wi.x,
        _wi.y
    ));

    /* Get polar coordinates */
    // We are setting phi = 0, as we don't care about the azimuth part.
    // We set theta as our polar angle
    float theta = utils::toPolar(wi).phi;

    /* Step 2: simulate P22_{wi}(slope.x, slope.y, 1, 1) */
    float slope = sampleMicrofacetDistr11(theta);

    /* Step 4: unstretch */
    slope *= alpha;

    /* Step 5: compute normal */
    // float normalization = 1.0f / std::sqrt(slope.x * slope.x
    //                                        + slope.y * slope.y + 1.0f);
    // return Normal(
    //     -slope.x * normalization,
    //     -slope.y * normalization,
    //     normalization
    // );

    return utils::normalize(Vec2f(slope, 1.0f));
}

// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/bsdfs/microfacet.h#L477
float microfacetDistrSmithG1(const Vec2f &v, const Vec2f &m, float alpha) {
    /* Ensure consistent orientation (can't see the back
       of the microfacet from the front and vice versa) */
    if (utils::dot(v, m) * utils::cosTheta(v) <= 0)
        return 0.0f;

    /* Perpendicular incidence -- no shadowing/masking */
    float tanTheta = std::abs(utils::tanTheta(v));
    if (tanTheta == 0.0f)
        return 1.0f;

    // EGGX
    float root = alpha * tanTheta;
    float hypot2 = sqrt(1.0f * 1.0f + root * root);
    return 2.0f / (1.0f + hypot2);
}

// https://github.com/mitsuba-renderer/mitsuba/blob/10af06f365886c1b6dd8818e0a3841078a62f283/src/bsdfs/roughconductor.cpp#L323
Polar MicrofacetBRDF::sample(const Polar &wi) const {
    assert(utils::cosTheta(wi) >= 0.0f);
    // if (((bRec.component != -1 && bRec.component != 0) ||
    //     !(bRec.typeMask & EGlossyReflection)))
    //     return Spectrum(0.0f);

    Vec2f wiVec = utils::toVec(wi);
    Vec2f m = sampleMicrofacetDistr(wiVec, mAlpha);

    /* Perfect specular reflection based on the microfacet normal */
    Vec2f wo = utils::reflect(utils::toVec(wi), m);
    assert(utils::cosTheta(wo) >= 0.0f);

    return utils::toPolar(wo);

    float F = fresnelConductorExact(utils::dot(wiVec, m), mEta, mK)
              * mSpecularReflectance;

    float weight = microfacetDistrSmithG1(wo, m, mAlpha);

    float spectrum = F * weight;
    // return F * weight;
}

// float MicrofacetBRDF::eval(const Polar &wi, const Polar &wo) const {
//     float F = fresnelConductorExact(utils::dot(wiVec, m), mEta, mK)
//               * mSpecularReflectance;
//
//     float weight = microfacetDistrSmithG1(wo, m, mAlpha);
//
//     return F * weight;
//
// }

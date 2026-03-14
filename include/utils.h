//
// Created by Saman on 29.01.26.
//

#ifndef PIS_UTILS_H
#define PIS_UTILS_H

#include "coords.h"

namespace utils {
    float safeSqrt(float x);

    /// Vec2f
    Polar toPolar(const Vec2f &);

    Vec2f reflect(const Vec2f &v, const Vec2f &axis);

    float dot(const Vec2f &v1, const Vec2f &v2);

    Vec2f normalize(const Vec2f &v);

    float cosTheta(const Vec2f &v);

    float sinTheta(const Vec2f &v);

    float tanTheta(const Vec2f &v);

    /// Vec3f
    Spherical toSpherical(const Vec3f &);

    float dot(const Vec3f &v1, const Vec3f &v2);

    Vec3f normalize(const Vec3f &v);

    float cosTheta(const Vec3f &v);

    float sinTheta(const Vec3f &v);

    float cosPhi(const Vec3f &v);

    float sinPhi(const Vec3f &v);

    /// Polar
    Vec2f toVec(const Polar &);

    Polar reflect(const Polar &p, const Polar &axis);

    float cosTheta(const Polar &p);

    float sinTheta(const Polar &p);

    float tanTheta(const Polar &p);

    Polar normalize(const Polar &p);

    /// Spherical
    Vec3f toVec(const Spherical &);
}

#endif //PIS_UTILS_H

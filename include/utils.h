//
// Created by Saman on 29.01.26.
//

#ifndef PIS_UTILS_H
#define PIS_UTILS_H

#include "coords.h"

namespace utils {
    // float cosTheta(float);

    Polar toPolar(const Vec2f &);

    Vec2f toVec(const Polar &);

    Spherical toSpherical(const Vec3f &);

    Vec3f toVec(const Spherical &);

    Polar reflect(const Polar &p, const Polar &axis);

    Vec2f reflect(const Vec2f &v, const Vec2f &axis);

    float dot(const Vec2f &v1, const Vec2f &v2);

    float dot(const Vec3f &v1, const Vec3f &v2);

    float cosTheta(const Vec2f &v);

    float cosTheta(const Polar &p);

    float sinTheta(const Vec2f &v);

    float sinTheta(const Polar &p);

    float tanTheta(const Vec2f &v);

    float tanTheta(const Polar &p);

    Vec2f normalize(const Vec2f &v);

    Polar normalize(const Polar &p);

    float safe_sqrt(float x);
}

#endif //PIS_UTILS_H

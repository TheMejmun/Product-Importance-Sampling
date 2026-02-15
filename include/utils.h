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
}

#endif //PIS_UTILS_H

//
// Created by Saman on 29.01.26.
//

#include   "utils.h"

#include <assert.h>
#include <cmath>

Polar utils::toPolar(const Vec2f &vec) {
    const float r = sqrt(vec.x * vec.x + vec.y * vec.y);
    const float phi = atan2(vec.y, vec.x);
    return {r,phi };
}

Vec2f utils::toVec(const Polar &polar) {
    const float x = polar.r * cos(polar.phi);
    const float y = polar.r * sin(polar.phi);
    return {x, y};
}

Spherical utils::toSpherical(const Vec3f &vec) {
const float r = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    const float theta = acos(vec.z / r);
    const float sign  = vec.y < 0 ? -1 : 1;
    // undefined for x = 0 && y = 0
    assert(sqrt(vec.x * vec.x + vec.y * vec.y) != 0);
    const float phi = sign * acos(vec.x / sqrt(vec.x * vec.x + vec.y * vec.y));
    return {r, theta, phi};
}
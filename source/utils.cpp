//
// Created by Saman on 29.01.26.
//

#include   "utils.h"

#include <cassert>

Polar utils::toPolar(const Vec2f &vec) {
    const float r = sqrt(vec.x * vec.x + vec.y * vec.y);
    const float phi = atan2(vec.y, vec.x);
    return {r, phi};
}

Vec2f utils::toVec(const Polar &polar) {
    const float x = polar.r * cos(polar.phi);
    const float y = polar.r * sin(polar.phi);
    return {x, y};
}

Spherical utils::toSpherical(const Vec3f &vec) {
    const float r = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    const float theta = acos(vec.z / r);
    const float sign = vec.y < 0 ? -1 : 1;
    // undefined for x = 0 && y = 0
    assert(sqrt(vec.x * vec.x + vec.y * vec.y) != 0);
    const float phi = sign * acos(vec.x / sqrt(vec.x * vec.x + vec.y * vec.y));
    return {r, theta, phi};
}

// TODO TEST
Polar utils::reflect(const Polar &p, const Polar &axis) {
    float phi = p.phi + (axis.phi - p.phi) * 2;
    if (phi >= 2 * M_PI) phi -= 2 * M_PI;
    else if (phi < 0) phi += 2 * M_PI;
    return {p.r, phi};
}

// TODO implement without converting to polar coords
Vec2f utils::reflect(const Vec2f &v, const Vec2f &n) {
    return (n * (2 * dot(v, n))) - v;
}

Vec3f utils::toVec(const Spherical &) {
    throw std::runtime_error("Not implemented");
}

float utils::dot(const Vec2f &v1, const Vec2f &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

float utils::dot(const Vec3f &v1, const Vec3f &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float utils::cosTheta(const Vec2f &v) {
    return normalize(v).y;
}

float utils::cosTheta(const Polar &p) {
    // Instead of calculating the cosine against the normal, we take the sine of the angle itself
    // https://en.wikipedia.org/wiki/Sine_and_cosine#/media/File:Sine_cosine_one_period.svg
    return cos(p.phi - static_cast<float>(0.5 * M_PI));
}

float utils::sinTheta(const Vec2f &v) {
    float sinTheta2 = 1.0f - v.y * v.y;
    if (sinTheta2 <= 0.0f)
        return 0.0f;
    return sqrt(sinTheta2);
}

float utils::sinTheta(const Polar &p) {
    return sin(p.phi - static_cast<float>(0.5 * M_PI));
}

float utils::tanTheta(const Vec2f &v) {
    return sinTheta(v) / cosTheta(v);
}

float utils::tanTheta(const Polar &p) {
    return sinTheta(p) / cosTheta(p);
}

Vec2f utils::normalize(const Vec2f &v) {
    const float l = sqrt(v.x * v.x + v.y * v.y);
    assert(l > 0);
    return {v.x / l, v.y / l};
}

Polar utils::normalize(const Polar &p) {
    return {1.0f, p.phi};
}

float utils::safe_sqrt(float x) {
    return x < 0 ? 0 : sqrt(x);
}

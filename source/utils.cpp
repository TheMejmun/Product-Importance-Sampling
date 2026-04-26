//
// Created by Saman on 29.01.26.
//

#include   "utils.h"

#include <cassert>
#include <random>
#include <stdexcept>

namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<double> randDistr(0.0, 1.0);
}

double utils::safeSqrt(double x) {
    return x < 0 ? 0 : sqrt(x);
}

/// Vec2f
Polar utils::toPolar(const Vec2f &vec) {
    const double r = sqrt(vec.x * vec.x + vec.y * vec.y);
    const double phi = atan2(vec.y, vec.x);
    return {r, phi};
}

// TODO implement without converting to polar coords
Vec2f utils::reflect(const Vec2f &v, const Vec2f &n) {
    return (n * (2 * dot(v, n))) - v;
}

double utils::dot(const Vec2f &v1, const Vec2f &v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

Vec2f utils::normalize(const Vec2f &v) {
    const double l = sqrt(v.x * v.x + v.y * v.y);
    assert(l > 0);
    return {v.x / l, v.y / l};
}

double utils::cosTheta(const Vec2f &v) {
    return normalize(v).y;
}

double utils::sinTheta(const Vec2f &v) {
    double sinTheta2 = 1.0 - v.y * v.y;
    if (sinTheta2 <= 0.0)
        return 0.0;
    return sqrt(sinTheta2);
}

double utils::tanTheta(const Vec2f &v) {
    return sinTheta(v) / cosTheta(v);
}

/// Vec3f
Spherical utils::toSpherical(const Vec3f &vec) {
    const double r = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    const double theta = acos(vec.z / r);
    const double sign = vec.y < 0 ? -1 : 1;
    // undefined for x = 0 && y = 0
    assert(sqrt(vec.x * vec.x + vec.y * vec.y) != 0);
    const double phi = sign * acos(vec.x / sqrt(vec.x * vec.x + vec.y * vec.y));
    return {r, theta, phi};
}

Vec3f utils::reflect(const Vec3f &v, const Vec3f &axis) {
    return axis * (2 * dot(axis, v)) - v;
}

double utils::dot(const Vec3f &v1, const Vec3f &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3f utils::normalize(const Vec3f &v) {
    const double l = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    assert(l > 0);
    return {v.x / l, v.y / l, v.z / l};
}

double utils::cosTheta(const Vec3f &v) {
    return normalize(v).z;
}

double utils::sinTheta(const Polar &p) {
    return sin(p.phi - static_cast<double>(0.5 * M_PI));
}

/** \brief Give a unit direction, this function returns the cosine of the
 * azimuth in a reference spherical coordinate system (see the \ref Frame
 * description)
 */
double utils::cosPhi(const Vec3f &v) {
    double sin_theta_2 = std::pow(sinTheta(v), 2.0),
            inv_sin_theta = 1.0 / sinTheta(v);
    return std::abs(sin_theta_2) <= 4.0 * 1e-4
               ? 1.0
               : std::min(std::max(v.x * inv_sin_theta, -1.0), 1.0);
}

/** \brief Give a unit direction, this function returns the sine of the
 * azimuth in a reference spherical coordinate system (see the \ref Frame
 * description)
 */
double utils::sinPhi(const Vec3f &v) {
    double sin_theta_2 = std::pow(sinTheta(v), 2.0),
            inv_sin_theta = 1.0 / sinTheta(v);
    return std::abs(sin_theta_2) <= 4.0 * 1e-4
               ? 0.0
               : std::min(std::max(v.y * inv_sin_theta, -1.0), 1.0);
}

/// Polar

Vec2f utils::toVec(const Polar &polar) {
    const double x = polar.r * cos(polar.phi);
    const double y = polar.r * sin(polar.phi);
    return {x, y};
}

// TODO TEST
Polar utils::reflect(const Polar &p, const Polar &axis) {
    double phi = p.phi + (axis.phi - p.phi) * 2;
    if (phi >= 2 * M_PI) phi -= 2 * M_PI;
    else if (phi < 0) phi += 2 * M_PI;
    return {p.r, phi};
}

double utils::cosTheta(const Polar &p) {
    // Instead of calculating the cosine against the normal, we take the sine of the angle itself
    // https://en.wikipedia.org/wiki/Sine_and_cosine#/media/File:Sine_cosine_one_period.svg
    return cos(p.phi - (0.5 * M_PI));
}

double utils::sinTheta(const Vec3f &v) {
    return std::sqrt(std::pow(v.x, 2.0) + std::pow(v.y, 2.0));
}

double utils::tanTheta(const Polar &p) {
    return sinTheta(p) / cosTheta(p);
}

Polar utils::normalize(const Polar &p) {
    return {1.0, p.phi};
}

/// Spherical
Vec3f utils::toVec(const Spherical &s) {
    return {
        s.r * sin(s.theta) * cos(s.phi),
        s.r * sin(s.theta) * sin(s.phi),
        s.r * cos(s.theta)
    };
}

Vec3f utils::hemisphereSample() {
    while (true) {
        const Vec3f sampleUnnormalized(
            randDistr(randEng) * 2 - 1,
            randDistr(randEng) * 2 - 1,
            randDistr(randEng)
        );
        const double mag2 = sampleUnnormalized.x * sampleUnnormalized.x +
                           sampleUnnormalized.y * sampleUnnormalized.y +
                           sampleUnnormalized.z * sampleUnnormalized.z;
        if (mag2 <= 1.0) {
            return normalize(sampleUnnormalized);
        }
    }
}

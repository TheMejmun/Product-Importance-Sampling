//
// Created by Saman on 29.01.26.
//

#ifndef PIS_UTILS_H
#define PIS_UTILS_H

#include <iostream>
#include <ostream>

#include "coords.h"
#include <stdio.h>

namespace utils {
    double safeSqrt(double x);

    template<typename T>
    T mse(const T &mean, const std::vector<T> &samples) {
        T sum{};
        // std::cout << "Zero: " << sum << std::endl;
        for (const T &sample: samples) {
            // std::cout << "Sample: " << sample << std::endl;
            sum += (sample - mean) * (sample - mean);
        }
        return sum / static_cast<double>(samples.size());
    }

    /// Vec2f
    Polar toPolar(const Vec2f &);

    Vec2f reflect(const Vec2f &v, const Vec2f &axis);

    double dot(const Vec2f &v1, const Vec2f &v2);

    Vec2f normalize(const Vec2f &v);

    double cosTheta(const Vec2f &v);

    double sinTheta(const Vec2f &v);

    double tanTheta(const Vec2f &v);

    /// Vec3f
    Spherical toSpherical(const Vec3f &);

    Vec3f reflect(const Vec3f &v, const Vec3f &axis);

    double dot(const Vec3f &v1, const Vec3f &v2);

    Vec3f normalize(const Vec3f &v);

    double cosTheta(const Vec3f &v);

    double sinTheta(const Vec3f &v);

    double cosPhi(const Vec3f &v);

    double sinPhi(const Vec3f &v);

    /// Polar
    Vec2f toVec(const Polar &);

    Polar reflect(const Polar &p, const Polar &axis);

    double cosTheta(const Polar &p);

    double sinTheta(const Polar &p);

    double tanTheta(const Polar &p);

    Polar normalize(const Polar &p);

    /// Spherical
    Vec3f toVec(const Spherical &);

    Vec3f hemisphereSample();

    inline double lerp(const double v0, const double v1, const double t) {
        return v0 + t * (v1 - v0);
    }
}

#endif //PIS_UTILS_H

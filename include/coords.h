//
// Created by Saman on 29.01.26.
//

#ifndef PIS_COORDS_H
#define PIS_COORDS_H
#include <valarray>

struct Vec2f {
    float x, y;
};

struct Vec3f {
    float x, y, z;
};

struct Polar {
    float r, phi;
    // Instead of calculating the cosine against the normal, we take the sine of the angle itself
    // https://en.wikipedia.org/wiki/Sine_and_cosine#/media/File:Sine_cosine_one_period.svg
    [[nodiscard]] float cosTheta() const { return sin(phi); }
};

// polar angle theta
// azimuth angle phi
struct Spherical {
    float r, theta, phi;
};


#endif //PIS_COORDS_H

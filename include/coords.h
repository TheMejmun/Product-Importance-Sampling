//
// Created by Saman on 29.01.26.
//

#ifndef PIS_COORDS_H
#define PIS_COORDS_H
#include <valarray>

struct Vec2f {
    float x, y;

    Vec2f operator+(const Vec2f &rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    Vec2f operator-(const Vec2f &rhs) const {
        return {x - rhs.x, y - rhs.y};
    }

    Vec2f operator*(float rhs) const {
        return {x * rhs, y * rhs};
    }
};

typedef Vec2f Normal2f;
typedef Vec2f Point2f;


struct Vec3f {
    float x, y, z;
};

typedef Vec3f Normal3f;

struct Polar {
    float r, phi;
};

// polar angle theta
// azimuth angle phi
struct Spherical {
    float r, theta, phi;
};


#endif //PIS_COORDS_H

//
// Created by Saman on 29.01.26.
//

#ifndef PIS_COORDS_H
#define PIS_COORDS_H
#include <valarray>

struct Vec2f {
    double x, y;

    Vec2f operator+(const Vec2f &rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    Vec2f operator-(const Vec2f &rhs) const {
        return {x - rhs.x, y - rhs.y};
    }

    Vec2f operator*(double rhs) const {
        return {x * rhs, y * rhs};
    }
};

typedef Vec2f Normal2f;
typedef Vec2f Point2f;


struct Vec3f {
    double x, y, z;

    Vec3f operator+(const Vec3f &rhs) const {
        return {x + rhs.x, y + rhs.y, z + rhs.z};
    }

    Vec3f operator-(const Vec3f &rhs) const {
        return {x - rhs.x, y - rhs.y, z - rhs.z};
    }

    Vec3f operator*(double rhs) const {
        return {x * rhs, y * rhs, z * rhs};
    }

    Vec3f operator*(Vec3f rhs) const {
        return {x * rhs.x, y * rhs.y, z * rhs.z};
    }

    Vec3f operator/(double rhs) const {
        return {x / rhs, y / rhs, z / rhs};
    }
};

typedef Vec3f Normal3f;

struct Polar {
    double r, phi;
};

// polar angle theta
// azimuth angle phi
struct Spherical {
    double r, theta, phi;
};


#endif //PIS_COORDS_H

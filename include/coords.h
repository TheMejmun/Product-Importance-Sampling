//
// Created by Saman on 29.01.26.
//

#ifndef PIS_COORDS_H
#define PIS_COORDS_H

struct Vec2f {
    float x, y;
};

struct Vec3f {
    float x, y, z;
};

struct Polar {
    float r, phi;
};

// polar angle theta
// azimuth angle phi
struct Spherical {
    float r, theta, phi;
};


#endif //PIS_COORDS_H

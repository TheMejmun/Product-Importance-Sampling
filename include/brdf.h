//
// Created by Saman on 29.01.26.
//

#ifndef PIS_BRDF_H
#define PIS_BRDF_H
#include "coords.h"

class BRDF {
protected:
    BRDF() = default;

    ~BRDF() = default;

public:
    // Should return the "color" -> in this case monochrome brightness
    virtual float eval(const Polar &wi, const Polar &wo) = 0;

    virtual float pdf(const Polar &wi, const Polar &wo) = 0;

    virtual Polar sample(const Polar &wi) = 0;
};

#endif //PIS_BRDF_H

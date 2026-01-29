//
// Created by Saman on 29.01.26.
//

#ifndef PIS_DIFFUSE_BRDF_H
#define PIS_DIFFUSE_BRDF_H

#include "brdf.h"

class DiffuseBRDF final : public BRDF {
public:
    float eval(const Polar &wi, const Polar &wo) override;

    float pdf(const Polar &wi, const Polar &wo) override;

    Polar sample(const Polar &wi) override;
};

#endif //PIS_DIFFUSE_BRDF_H

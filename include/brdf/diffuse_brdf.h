//
// Created by Saman on 29.01.26.
//

#ifndef PIS_DIFFUSE_BRDF_H
#define PIS_DIFFUSE_BRDF_H

#include "brdf.h"

class DiffuseBRDF final : public BRDF {
public:
    [[nodiscard]] float eval(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] float pdf(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] Polar sample(const Polar &wi) const override;
};

#endif //PIS_DIFFUSE_BRDF_H

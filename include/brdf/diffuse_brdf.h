//
// Created by Saman on 29.01.26.
//

#ifndef PIS_DIFFUSE_BRDF_H
#define PIS_DIFFUSE_BRDF_H

#include "brdf.h"

class DiffuseBRDF final : public BRDF {
public:
    [[nodiscard]] double eval(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] double pdf(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] Polar sample(const Polar &wi) const override;

    [[nodiscard]] double pdf(const Polar &wi, const Polar &wo, uint32_t nodeIndex) const override;

    [[nodiscard]] Polar sample(const Polar &wi, uint32_t nodeIndex) const override;

    [[nodiscard]] double eval(const Vec3f &wi, const Vec3f &wo) const override;

    [[nodiscard]] double pdf(const Vec3f &wi, const Vec3f &wo) const override;

    [[nodiscard]] Vec3f sample(const Vec3f &wi) const override;

    [[nodiscard]] const char* name()const override;

    void calculateMapping(const MSTree &msTree, const Polar &wi) override;
};

#endif //PIS_DIFFUSE_BRDF_H

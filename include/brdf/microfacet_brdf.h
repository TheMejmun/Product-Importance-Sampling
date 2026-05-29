//
// Created by Saman on 08.04.26.
//

#ifndef PIS_MICROFACET_BRDF_H
#define PIS_MICROFACET_BRDF_H

#include "brdf.h"
#include "mts/microfacet.h"

class MicrofacetBRDF final : public BRDF {
public:
    [[nodiscard]] double eval(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] double pdf(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] Polar sample(const Polar &wi) const override;

    [[nodiscard]] double eval(const Vec3f &wi, const Vec3f &wo) const override;

    [[nodiscard]] double pdf(const Vec3f &wi, const Vec3f &wo) const override;

    [[nodiscard]] Vec3f sample(const Vec3f &wi) const override;

    [[nodiscard]] const char * name() const override;

private:
    mts::MicrofacetDistribution mDistribution{0.1f};
};

#endif //PIS_MICROFACET_BRDF_H
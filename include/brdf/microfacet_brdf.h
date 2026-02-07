//
// Created by Saman on 31.12.25.
//

#ifndef PIS_MICROFACET_BRDF_H
#define PIS_MICROFACET_BRDF_H
#include "brdf.h"

class MicrofacetBRDF final : public BRDF {
public:
    [[nodiscard]] float eval(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] float pdf(const Polar &wi, const Polar &wo) const override;

    [[nodiscard]] Polar sample(const Polar &wi) const override;
};

#endif //PIS_MICROFACET_BRDF_H

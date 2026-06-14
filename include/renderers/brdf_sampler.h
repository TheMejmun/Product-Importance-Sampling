//
// Created by Saman on 05.02.26.
//

#ifndef PIS_BRDF_SAMPLING_H
#define PIS_BRDF_SAMPLING_H

#include "renderers/abstract_sampler.h"

class BRDFSampler final : public AbstractSampler {
public:
    [[nodiscard]] double sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                const Polar &wi) const override;

    [[nodiscard]] double sample3D(const BRDF &brdf, const Vec3f &wi) const;

    [[nodiscard]] double equal_samples3D(
        std::uint32_t iterations,
        const BRDF &brdf,
        const Vec3f &wi
    ) const;
};

#endif //PIS_BRDF_SAMPLING_H

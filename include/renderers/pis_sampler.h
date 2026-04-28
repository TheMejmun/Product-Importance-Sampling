//
// Created by Saman on 07.02.26.
//

#ifndef PIS_PI_SAMPLING_H
#define PIS_PI_SAMPLING_H

#include "renderers/abstract_sampler.h"

class PISSampler final : public AbstractSampler {
    [[nodiscard]] double sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                const Polar &wi) const override;
};

#endif //PIS_PI_SAMPLING_H

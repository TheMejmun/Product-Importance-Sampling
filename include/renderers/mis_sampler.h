//
// Created by Saman on 07.02.26.
//

#ifndef PIS_MI_SAMPLING_H
#define PIS_MI_SAMPLING_H

#include "renderers/abstract_sampler.h"

class MISSampler final : public AbstractSampler {
public:
    [[nodiscard]] double sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                const Polar &wi) const override;
};

#endif //PIS_MI_SAMPLING_H

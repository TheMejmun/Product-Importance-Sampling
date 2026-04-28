//
// Created by Saman on 05.02.26.
//

#ifndef PIS_BRDF_SAMPLING_H
#define PIS_BRDF_SAMPLING_H

#include "renderers/abstract_sampler.h"

class BRDFSampler final : public AbstractSampler {
    [[nodiscard]] double sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                                const Polar &wi) const override;
};

#endif //PIS_BRDF_SAMPLING_H

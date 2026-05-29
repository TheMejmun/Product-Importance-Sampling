//
// Created by Saman on 28.04.26.
//

#ifndef PIS_ABSTRACT_SAMPLER_H
#define PIS_ABSTRACT_SAMPLER_H

#include "brdf/brdf.h"
#include "light_source.h"
#include "multistage_tree.h"
#include <cstdint>
#include   <vector>

class AbstractSampler {
protected:
    AbstractSampler() = default;

    ~AbstractSampler() = default;

public:
    [[nodiscard]] double equal_samples(
        std::uint32_t iterations,
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar *wi
    ) const;

    [[nodiscard]] double mse(
        double reference,
        std::uint32_t iterations,
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar *wi
    ) const;

    [[nodiscard]] double equal_time(
        double seconds,
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar *wi
    ) const;

    [[nodiscard]] virtual double sample(
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar &wi
    ) const = 0;
};

#endif //PIS_ABSTRACT_SAMPLER_H

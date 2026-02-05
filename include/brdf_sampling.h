//
// Created by Saman on 05.02.26.
//

#ifndef PIS_BRDF_SAMPLING_H
#define PIS_BRDF_SAMPLING_H

#include "brdf.h"
#include "light_source.h"
#include <cstdint>
#include   <vector>

namespace sampling {
    float sample_brdf(
        std::uint32_t iterations,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar &wi
    );

    float sample_brdf(const BRDF &brdf, const std::vector<LightSource> &lightSources, const Polar &wi);
}

#endif //PIS_BRDF_SAMPLING_H

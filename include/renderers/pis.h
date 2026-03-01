//
// Created by Saman on 07.02.26.
//

#ifndef PIS_PI_SAMPLING_H
#define PIS_PI_SAMPLING_H

#include "light_source.h"
#include   <vector>

#include "multistage_tree.h"
#include "brdf/brdf.h"

namespace sampling {
    float pis(
        std::uint32_t iterations,
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar &wi
    );

    float pis(
        const MSTree &msTree,
        const BRDF &brdf,
        const std::vector<LightSource> &lightSources,
        const Polar &wi
    );
}
#endif //PIS_PI_SAMPLING_H
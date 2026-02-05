//
// Created by Saman on 04.02.26.
//

#ifndef PIS_LIGHT_SOURCE_H
#define PIS_LIGHT_SOURCE_H

#include <vector>

#include "coords.h"

struct LightSource {
    float start_angle;
    float end_angle;
    float intensity;
};

namespace sampling {
    float sample_lights(const std::vector<LightSource> &lightSources, const Polar &wo);
}

#endif //PIS_LIGHT_SOURCE_H
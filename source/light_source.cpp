//
// Created by Saman on 06.02.26.
//

#include "light_source.h"

#include <random>

// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_int_distribution<uint32_t> randIntDistr(0, std::numeric_limits<uint32_t>::max());
}


float sampling::sample_lights(const std::vector<LightSource> &lightSources, const Polar &wo) {
    std::vector<LightSource const *> reachableLights;
    for (const auto &light: lightSources) {
        if (light.start_angle <= wo.phi && light.end_angle >= wo.phi) {
            reachableLights.push_back(&light);
        }
    }
    // Randomly take one of them for the intensity
    float intensity = reachableLights.empty()
                          ? 0
                          : reachableLights[randIntDistr(randEng) % reachableLights.size()]->intensity;
    return intensity;
}

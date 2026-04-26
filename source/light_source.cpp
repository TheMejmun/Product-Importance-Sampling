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

double sample_lights_random(const std::vector<LightSource> &lightSources, const Polar &wo) {
    std::vector<LightSource const *> reachableLights;
    for (const auto &light: lightSources) {
        if (light.start_angle <= wo.phi && light.end_angle >= wo.phi) {
            reachableLights.push_back(&light);
        }
    }
    // Randomly take one of them for the intensity
    double intensity = reachableLights.empty()
                          ? 0
                          : reachableLights[randIntDistr(randEng) % reachableLights.size()]->intensity;
    return intensity;
}

double sample_lights_additive(const std::vector<LightSource> &lightSources, const Polar &wo) {
    double intensity = 0.0;
    for (const auto &light: lightSources) {
        if (light.start_angle <= wo.phi && light.end_angle >= wo.phi) {
            intensity += light.intensity;
        }
    }
    return intensity;
}

double sample_lights_average(const std::vector<LightSource> &lightSources, const Polar &wo) {
    double intensity = 0.0;
    size_t count = 0;
    for (const auto &light: lightSources) {
        if (light.start_angle <= wo.phi && light.end_angle >= wo.phi) {
            intensity += light.intensity;
            ++count;
        }
    }
    return intensity / static_cast<double>(count);
}

double sampling::intersect_lights(const std::vector<LightSource> &lightSources, const Polar &wo) {
    // The additive method makes it easiest to calculate analytical irradiance
    return sample_lights_additive(lightSources, wo);
}

//
// Created by Saman on 07.02.26.
//

#include "renderers/pis.h"

// TODO cosTheta wi
float sampling::pis(std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                    const std::vector<LightSource> &lightSources, const Polar &wi) {
    float color = 0.f;
    for (int i = 0; i < iterations; ++i) {
        color += pis(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<float>(iterations);
    return color;
}

// TODO cosTheta wo
float sampling::pis(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                    const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const float brdfIncoming = intersect_lights(lightSources, wo);
    const float brdfPdf = brdf.pdf(wi, wo);
    const float brdfColor = brdfIncoming * brdf.eval(wi, wo) / brdfPdf;

    const MSTSample sample = msTree.sample();
    const float mstIncoming = intersect_lights(lightSources, sample.wo);
    const float mstPdf = sample.pdf;
    const float mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    return 0.f;
}

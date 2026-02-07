//
// Created by Saman on 07.02.26.
//

#include "renderers/mis.h"

float sampling::mis(std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                    const std::vector<LightSource> &lightSources, const Polar &wi) {
    float color = 0.f;
    for (int i = 0; i < iterations; ++i) {
        color += mis(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<float>(iterations);
    return color;
}

float mix(float val0, float w0, float val1, float w1) {
    return (val0 * w0 + val1 * w1) / (w0 + w1);
}

float sampling::mis(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                    const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const float brdfIncoming = intersect_lights(lightSources, wo);
    const float brdfPdf = brdf.pdf(wi, wo);
    const float brdfColor = brdfIncoming * brdf.eval(wi, wo) / brdfPdf;

    const MSTSample sample = msTree.sample();
    const float mstIncoming = intersect_lights(lightSources, sample.wo);
    const float mstPdf = sample.pdf;
    const float mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    // (
    //   (color0 / pdf0) * pdf0 +
    //   (color1 / pdf1) * pdf1
    // ) / (pdf0 + pdf1)
    return mix(brdfColor, brdfPdf, mstColor, mstPdf);
}

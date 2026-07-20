//
// Created by Saman on 07.02.26.
//

#include "renderers/mis_sampler.h"

#include "utils.h"

double MISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wi) const {
    double color = 0.0;

    const Polar brdfWo = brdf.sample(wi);
    if (utils::cosTheta(brdfWo) >= 0.0) {
        const double brdfPdf = brdf.pdf(wi, brdfWo);
        if (brdfPdf > 0.0) {
            const double brdfIncoming = sampling::intersect_lights(lightSources, brdfWo);
            const double brdfFlux = brdfIncoming * brdf.eval(wi, brdfWo) * utils::cosTheta(brdfWo);
            color += brdfFlux / (brdfPdf + msTree.pdf(brdfWo));
        }
    }

    const MSTSample mstSample = msTree.sample();
    const double mstIncoming = sampling::intersect_lights(lightSources, mstSample.wo);
    const double mstFlux = mstIncoming * brdf.eval(wi, mstSample.wo) * utils::cosTheta(mstSample.wo);
    color += mstFlux / (mstSample.pdf + brdf.pdf(wi, mstSample.wo));

    return color;
}

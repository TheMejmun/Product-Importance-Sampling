//
// Created by Saman on 07.02.26.
//

#include "renderers/mis_sampler.h"

#include "utils.h"

double MISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wo) const {
    double color = 0.0;

    const Polar brdfWi = brdf.sample(wo);
    if (utils::cosTheta(brdfWi) >= 0.0) {
        const double brdfPdf = brdf.pdf(wo, brdfWi);
        if (brdfPdf > 0.0) {
            const double brdfIncoming = sampling::intersect_lights(lightSources, brdfWi);
            const double brdfFlux = brdfIncoming * brdf.eval(wo, brdfWi) * utils::cosTheta(brdfWi);
            color += brdfFlux / (brdfPdf + msTree.pdf(brdfWi));
        }
    }

    const MSTSample mstSample = msTree.sample();
    const double mstIncoming = sampling::intersect_lights(lightSources, mstSample.wi);
    const double mstFlux = mstIncoming * brdf.eval(wo, mstSample.wi) * utils::cosTheta(mstSample.wi);
    color += mstFlux / (mstSample.pdf + brdf.pdf(wo, mstSample.wi));

    return color;
}

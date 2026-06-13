//
// Created by Saman on 07.02.26.
//

#include "renderers/pis_sampler.h"

#include "utils.h"

// TODO cosTheta wi, wo
double PISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wi) const {
    const Polar wo = brdf.sample(wi);
    double brdfPdf = 0.0;
    double brdfColor = 0.0;
    if (utils::cosTheta(wo) >= 0.0) {
        const double pdf = brdf.pdf(wi, wo);
        if (pdf > 0.0) {
            const double brdfIncoming = sampling::intersect_lights(lightSources, wo);
            brdfPdf = pdf;
            brdfColor = brdfIncoming * brdf.eval(wi, wo) / pdf;
        }
    }

    const MSTSample sample = msTree.sample();
    const double mstIncoming = sampling::intersect_lights(lightSources, sample.wo);
    const double mstPdf = sample.pdf;
    const double mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    // TODO
    return 0.0;
}

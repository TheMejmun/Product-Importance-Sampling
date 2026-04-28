//
// Created by Saman on 07.02.26.
//

#include "renderers/pis_sampler.h"

// TODO cosTheta wi, wo
double PISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wi) const {
    const Polar wo = brdf.sample(wi);
    const double brdfIncoming = sampling::intersect_lights(lightSources, wo);
    const double brdfPdf = brdf.pdf(wi, wo);
    const double brdfColor = brdfIncoming * brdf.eval(wi, wo) / brdfPdf;

    const MSTSample sample = msTree.sample();
    const double mstIncoming = sampling::intersect_lights(lightSources, sample.wo);
    const double mstPdf = sample.pdf;
    const double mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    // TODO
    return 0.0;
}

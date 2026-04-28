//
// Created by Saman on 07.02.26.
//

#include "renderers/pis.h"

// TODO cosTheta wi
double sampling::pis(std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                     const std::vector<LightSource> &lightSources, const Polar &wi) {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += pis(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<double>(iterations);
    return color;
}

double sampling::pis_mse(double reference, std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                         const std::vector<LightSource> &lightSources, const Polar &wi) {
    double mse = 0.0;
    for (int i = 0; i < iterations; ++i) {
        const double color = pis(msTree, brdf, lightSources, wi);
        mse += (color - reference) * (color - reference);
    }
    mse /= static_cast<double>(iterations);
    return mse;
}

double sampling::pis(double seconds, const MSTree &msTree, const BRDF &brdf,
                     const std::vector<LightSource> &lightSources, const Polar &wi) {
    double color = 0.0;
    int iterations = 0;
    time_t start;
    time(&start);
    for (;;) {
        const double res = pis(msTree, brdf, lightSources, wi);
        time_t now;
        time(&now);
        const double duration = difftime(now, start);
        if (duration > seconds) {
            break;
        }
        color += res;
        ++iterations;
    }
    printf("\t%d samples in %f seconds\t", iterations, seconds);
    color /= static_cast<double>(iterations);
    return color;
}

// TODO cosTheta wo
double sampling::pis(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                     const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const double brdfIncoming = intersect_lights(lightSources, wo);
    const double brdfPdf = brdf.pdf(wi, wo);
    const double brdfColor = brdfIncoming * brdf.eval(wi, wo) / brdfPdf;

    const MSTSample sample = msTree.sample();
    const double mstIncoming = intersect_lights(lightSources, sample.wo);
    const double mstPdf = sample.pdf;
    const double mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    return 0.0;
}

//
// Created by Saman on 07.02.26.
//

#include "renderers/mis.h"

// TODO cosTheta wi
double sampling::mis(std::uint32_t iterations, const MSTree &msTree, const BRDF &brdf,
                     const std::vector<LightSource> &lightSources, const Polar &wi) {
    double color = 0.0;
    for (int i = 0; i < iterations; ++i) {
        color += mis(msTree, brdf, lightSources, wi);
    }
    color /= static_cast<double>(iterations);
    return color;
}

double sampling::mis(double seconds, const MSTree &msTree, const BRDF &brdf,
                     const std::vector<LightSource> &lightSources, const Polar &wi) {
    double color = 0.0;
    int iterations = 0;
    time_t start;
    time(&start);
    for (;;) {
        const double res = mis(msTree, brdf, lightSources, wi);
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

double mix(double val0, double w0, double val1, double w1) {
    return (val0 * w0 + val1 * w1) / (w0 + w1);
}

// TODO cosTheta wo
double sampling::mis(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                     const Polar &wi) {
    const Polar wo = brdf.sample(wi);
    const double brdfIncoming = intersect_lights(lightSources, wo);
    const double brdfPdf = brdf.pdf(wi, wo);
    const double brdfColor = brdfIncoming * brdf.eval(wi, wo) / brdfPdf;

    const MSTSample sample = msTree.sample();
    const double mstIncoming = intersect_lights(lightSources, sample.wo);
    const double mstPdf = sample.pdf;
    const double mstColor = mstIncoming * brdf.eval(wi, sample.wo) / mstPdf;

    // (
    //   (color0 / pdf0) * pdf0 +
    //   (color1 / pdf1) * pdf1
    // ) / (pdf0 + pdf1)
    return mix(brdfColor, brdfPdf, mstColor, mstPdf);
}

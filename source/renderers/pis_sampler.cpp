//
// Created by Saman on 07.02.26.
//

#include "renderers/pis_sampler.h"

#include "utils.h"


// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<double> randDistr(0.0, 1.0);
}

// TODO cache msTreeCopy 
double PISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wi) const {
    MSTree msTreeCopy = msTree.copy();

    // Estimate pdf integral
    std::vector<double> brdfPdfs(msTree.getPdfs().size());
    double totalBrdfPdf = 0.0;
    double totalProductPdf = 0.0;
    for (uint32_t i = 0; i < msTreeCopy.getNodes().size(); ++i) {
        const Node &node = msTreeCopy.getNodes()[i];

        double brdfPdf = 0.0;
        for (int j = 0; j < mMontecarloSamples; ++j) {
            const Polar wo{1.0, node.startBoundary + randDistr(randEng) * node.width()};
            brdfPdf += brdf.pdf(wi, wo);
        }
        brdfPdfs[i] = brdfPdf / mMontecarloSamples;
        totalBrdfPdf += brdfPdfs[i];

        // Combine direct light data structure pdf
        // with brdf pdf integral
        msTreeCopy.getPdfs()[i] *= brdfPdfs[i];
        totalProductPdf += msTreeCopy.getPdfs()[i];
    }
    // Normalize -> integrate to 1
    for (uint32_t i = 0; i < msTreeCopy.getNodes().size(); ++i) {
        msTreeCopy.getPdfs()[i] /= totalProductPdf;
        brdfPdfs[i] /= totalBrdfPdf;
    }

    // double msTreePdfTotal = 0.0;
    // for (const auto &pdf: msTree.getPdfs()) { msTreePdfTotal += pdf; }
    // printf("msTreePdfTotal\t%f\n", msTreePdfTotal);
    // double msTreeCopyPdfTotal = 0.0;
    // for (const auto &pdf: msTreeCopy.getPdfs()) { msTreeCopyPdfTotal += pdf; }
    // printf("msTreeCopyPdfTotal\t%f\n", msTreeCopyPdfTotal);
    // double brdfPdfTotal = 0.0;
    // for (const auto &pdf: brdfPdfs) { brdfPdfTotal += pdf; }
    // printf("brdfPdfTotal\t%f\n", brdfPdfTotal);

    // Get a sampling region
    const uint32_t nodeIndex = msTreeCopy.sampleNodes(); // region from the data structure according to distribution
    const double nodePdf = msTreeCopy.getPdfs()[nodeIndex]; // combined light + brdf pdf
    const double brdfPdf = brdfPdfs[nodeIndex]; // brdf pdf integral

    if (nodePdf == 0.0) { return 0.0; }
    if (brdfPdf == 0.0) { return 0.0; }

    // Sample the brdf in that region
    const Polar wo = brdf.sample(wi, nodeIndex);
    const double pdf = (brdf.pdf(wi, wo) / brdfPdf) * nodePdf;

    if (pdf == 0.0) { return 0.0; }

    const double incoming = sampling::intersect_lights(lightSources, wo);
    return incoming * brdf.eval(wi, wo) * utils::cosTheta(wo) / pdf;
}

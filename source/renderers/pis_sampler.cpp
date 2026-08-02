//
// Created by Saman on 07.02.26.
//

#include "renderers/pis_sampler.h"

#include "utils.h"


// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
}

double PISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wi) const {
    const std::vector<double> &lightPdfs = msTree.getPdfs();

    // Product distribution over nodes:
    // light flux share x brdf probability mass inside the node
    std::vector<double> productPdfs(lightPdfs.size());
    double totalProductPdf = 0.0;
    for (uint32_t nodeIndex = 0; nodeIndex < productPdfs.size(); ++nodeIndex) {
        productPdfs[nodeIndex] = lightPdfs[nodeIndex] * brdf.nodeSamplingProbability(nodeIndex);
        totalProductPdf += productPdfs[nodeIndex];
    }
    if (totalProductPdf <= 0.0) { return 0.0; }

    // Get a sampling region
    const uint32_t nodeIndex = std::discrete_distribution<uint32_t>(productPdfs.begin(), productPdfs.end())(randEng);
    const double nodePdf = productPdfs[nodeIndex] / totalProductPdf;

    // Sample the brdf in that region
    const Polar wo = brdf.sample(wi, nodeIndex);
    if (utils::cosTheta(wo) < 0.0) { return 0.0; }
    const double pdf = brdf.pdf(wi, wo, nodeIndex) * nodePdf;
    if (pdf <= 0.0) { return 0.0; }

    const double incoming = sampling::intersect_lights(lightSources, wo);
    return incoming * brdf.eval(wi, wo) * utils::cosTheta(wo) / pdf;
}

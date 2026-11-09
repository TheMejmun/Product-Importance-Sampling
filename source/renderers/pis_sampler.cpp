//
// Created by Saman on 07.02.26.
//

#include "renderers/pis_sampler.h"

#include "utils.h"


// Anonymous namespace ensures internal linkage
namespace {
    thread_local std::random_device randDev;
    thread_local std::default_random_engine randEng(randDev());
}

double PISSampler::sample(const MSTree &msTree, const BRDF &brdf, const std::vector<LightSource> &lightSources,
                          const Polar &wo) const {
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
    const Polar wi = brdf.sample(wo, nodeIndex);
    if (utils::cosTheta(wi) < 0.0) { return 0.0; }
    const double pdf = brdf.pdf(wo, wi, nodeIndex) * nodePdf;
    if (pdf <= 0.0) { return 0.0; }

    const double incoming = sampling::intersect_lights(lightSources, wi);
    return incoming * brdf.eval(wo, wi) * utils::cosTheta(wi) / pdf;
}

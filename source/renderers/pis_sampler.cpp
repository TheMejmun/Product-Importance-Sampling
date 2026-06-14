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
    for (uint32_t i = 0; i < msTreeCopy.getNodes().size(); ++i) {
        const Node &node = msTreeCopy.getNodes()[i];

        double brdfPdf = 0.0;
        for (int j = 0; j < mMontecarloSamples; ++j) {
            const Polar wo{1.0, node.startBoundary + randDistr(randEng) * node.width()};
            brdfPdf += brdf.pdf(wi, wo);
        }
        msTreeCopy.getPdfs()[i] *= brdfPdf / mMontecarloSamples;
    }

    double totalBrdfPdf = 0.0;
    for (const double i: msTreeCopy.getPdfs()) {
        totalBrdfPdf += i;
    }
    for (double &i: msTreeCopy.getPdfs()) {
        i /= totalBrdfPdf;
    }

    // TODO do not just sample mstree
    // TODO sample brdf in limited area
    // TODO pdf of brdf should be adjusted to reflect limited domain -> multiply with mstree pdf
    const MSTSample sample = msTreeCopy.sample();
    const double incoming = sampling::intersect_lights(lightSources, sample.wo);
    return incoming * brdf.eval(wi, sample.wo) * utils::cosTheta(sample.wo) / sample.pdf;
}

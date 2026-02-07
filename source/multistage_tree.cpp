//
// Created by Saman on 29.12.25.
//

#include "multistage_tree.h"
#include <algorithm>
#include <cassert>
#include <fstream>
#include <random>

// Anonymous namespace ensures internal linkage
namespace {
    std::random_device randDev;
    std::default_random_engine randEng(randDev());
    std::uniform_real_distribution<float> randDistr(0.f, 1.f);
    std::uniform_int_distribution<uint32_t> randIntDistr(0, std::numeric_limits<uint32_t>::max());
}

void MSTree::add(const float position, const float value) {
    assert(position >= 0.f && position <= M_PI);
    mTotalFlux += value;
    mLightSamples.emplace_back(value, position);
}

void MSTree::compileRec(float leftBoundary, float rightBoundary) {
    float flux = 0.f;
    size_t count = 0;
    for (const auto &sample: mLightSamples) {
        if (sample.position < leftBoundary)
            continue;
        if (sample.position > rightBoundary)
            break;

        flux += sample.flux;
        ++count;
    }

    if (mTotalFlux > 0.0f && count > 1 && flux / mTotalFlux > SUBDIV_THRESHOLD) {
        assert(count > 1);
        const float midPoint = leftBoundary + ((rightBoundary - leftBoundary) / 2.f);
        //printf("subdividing [%f, %f] at %f\n", leftBoundary, rightBoundary, midPoint);
        compileRec(leftBoundary, midPoint);
        compileRec(midPoint, rightBoundary);
    } else {
        // printf("adding node [%f, %f] with flux %f\n", leftBoundary, rightBoundary, flux);
        mNodes.emplace_back(flux, leftBoundary, rightBoundary);
    }
}

void MSTree::compile() {
    mNodes.clear();
    printf("Total flux in MSTree %f\n", mTotalFlux);
    assert(mTotalFlux > 0.f);

    std::sort(mLightSamples.begin(), mLightSamples.end());
    compileRec(0.f, M_PI);

    printf("Compiled %lu samples into %lu nodes\n", mLightSamples.size(), mNodes.size());


    pdfs = std::vector<float>(mNodes.size());
    float newTotalFlux = 0.f;
    float pdfSum = 0.f;
    for (size_t i = 0; i < mNodes.size(); ++i) {
        pdfs[i] = pdf(mNodes[i]);
        pdfSum += pdfs[i];
        newTotalFlux += mNodes[i].flux;
    }
    printf("mTotalFlux %f -> %f\n", mTotalFlux, newTotalFlux);
    printf("PDF sum: %f\n", pdfSum);
}

const MSTSample &MSTree::sample() const {
    // Get a random Node weighted by its relative flux
    // TODO this would be faster in a binary tree
    const size_t index = std::discrete_distribution<size_t>(pdfs.begin(), pdfs.end())(randEng);
    const Node &sampleNode = mNodes[index];

    float position = sampleNode.leftBoundary + randDistr(randEng) * sampleNode.width();
    MSTSample result{
        Polar{1.f, position},
        pdfs[index] / sampleNode.width()
    };
    return {result};
}

float MSTree::pdf(const Node &node) const {
    float relativeFlux = node.flux / mTotalFlux;
    // float nodeAngle = node.width() / static_cast<float>(M_PI);
    return relativeFlux;
}

void MSTree::exportToCsv(const std::string &filename) {
    std::ofstream csv;
    csv.open(filename);
    csv << "index,flux,width,leftBound,rightBound,\n";
    for (size_t i = 0; i < mNodes.size(); ++i) {
        const Node &n = mNodes[i];
        csv << i << "," << n.flux << ",";
        csv << n.rightBoundary - n.leftBoundary << "," << n.leftBoundary << "," << n.rightBoundary << ",\n";
    }
    csv.close();
}

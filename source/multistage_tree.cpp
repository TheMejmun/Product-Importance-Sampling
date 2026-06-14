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
    std::uniform_real_distribution<double> randDistr(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> randIntDistr(0, std::numeric_limits<uint32_t>::max());
}

void MSTree::add(const double position, const double value) {
    assert(position >= 0.0 && position <= M_PI);
    mTotalFlux += value;
    mLightSamples.emplace_back(value, position);
}

void MSTree::compileRec(double leftBoundary, double rightBoundary) {
    double flux = 0.0;
    uint32_t count = 0;
    for (const auto &sample: mLightSamples) {
        if (sample.position < leftBoundary)
            continue;
        if (sample.position > rightBoundary)
            break;

        flux += sample.flux;
        ++count;
    }

    if (mTotalFlux > 0.0 && count > 1 && flux / mTotalFlux > SUBDIV_THRESHOLD) {
        assert(count > 1);
        const double midPoint = leftBoundary + ((rightBoundary - leftBoundary) / 2.0);
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
    assert(mTotalFlux > 0.0);

    std::sort(mLightSamples.begin(), mLightSamples.end());
    compileRec(0.0, M_PI);

    printf("Compiled %lu samples into %lu nodes\n", mLightSamples.size(), mNodes.size());


    mPdfs = std::vector<double>(mNodes.size());
    double newTotalFlux = 0.0;
    double pdfSum = 0.0;
    for (uint32_t i = 0; i < mNodes.size(); ++i) {
        mPdfs[i] = pdf(mNodes[i]);
        pdfSum += mPdfs[i];
        newTotalFlux += mNodes[i].flux;
    }
    printf("mTotalFlux %f -> %f\n", mTotalFlux, newTotalFlux);
    printf("PDF sum: %f\n", pdfSum);
}

 MSTSample MSTree::sample() const {
    // Get a random Node weighted by its relative flux
    // TODO this would be faster in a binary tree
    const uint32_t index = std::discrete_distribution<uint32_t>(mPdfs.begin(), mPdfs.end())(randEng);
    const Node &sampleNode = mNodes[index];

    double position = sampleNode.startBoundary + randDistr(randEng) * sampleNode.width();
    MSTSample result{
        Polar{1.0, position},
        mPdfs[index] / sampleNode.width()
    };
    return {result};
}

double MSTree::pdf(const Node &node) const {
    double relativeFlux = node.flux / mTotalFlux;
    // double nodeAngle = node.width() / static_cast<double>(M_PI);
    return relativeFlux;
}

void MSTree::exportToCsv(const std::string &filename) {
    std::ofstream csv;
    csv.open(filename);
    csv << "index,flux,width,leftBound,rightBound,\n";
    for (uint32_t i = 0; i < mNodes.size(); ++i) {
        const Node &n = mNodes[i];
        csv << i << "," << n.flux << ",";
        csv << n.endBoundary - n.startBoundary << "," << n.startBoundary << "," << n.endBoundary << ",\n";
    }
    csv.close();
}

std::vector<Node> & MSTree::getNodes() {
    return mNodes;
}

std::vector<double> &MSTree::getPdfs() {
    return mPdfs;
}

MSTree MSTree::copy() const {
    MSTree copy{};
    copy.mTotalFlux = mTotalFlux;
    copy.mNodes = mNodes;
    copy.mLightSamples = mLightSamples;
    copy.mPdfs = mPdfs;
    return copy;
}
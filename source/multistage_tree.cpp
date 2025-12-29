//
// Created by Saman on 29.12.25.
//

#include "multistage_tree.h"
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <random>

std::random_device randDev;
std::default_random_engine randEng(randDev());
std::uniform_real_distribution<float> randDistr(0.f, 1.f);

void MSTree::add(const float position, const float value) {
    assert(position >= 0.f && position <= 1.f);
    mTotalFlux += value;
    mSamples.emplace_back(value, position);
}

void MSTree::compileRec(float leftBoundary, float rightBoundary) {
    float flux = 0.f;
    size_t count = 0;
    for (const auto &sample: mSamples) {
        if (sample.position < leftBoundary)
            continue;
        if (sample.position > rightBoundary)
            break;

        flux += sample.flux;
        ++count;
    }

    if (flux / mTotalFlux > SUBDIV_THRESHOLD) {
        assert(count > 1);
        const float midPoint = leftBoundary + ((rightBoundary - leftBoundary) / 2.f);
        //printf("subdividing [%f, %f] at %f\n", leftBoundary, rightBoundary, midPoint);
        compileRec(leftBoundary, midPoint);
        compileRec(midPoint, rightBoundary);
    } else {
        printf("adding node [%f, %f] with flux %f\n", leftBoundary, rightBoundary, flux);
        mNodes.emplace_back(flux, leftBoundary, rightBoundary);
    }
}

void MSTree::compile() {
    mNodes.clear();

    std::sort(mSamples.begin(), mSamples.end());
    compileRec(0.f, 1.f);

    printf("Compiled %lu samples into %lu nodes", mSamples.size(), mNodes.size());
}

const Node &MSTree::sample() const {
    float rand = randDistr(randEng);
    for (const auto &node: mNodes) {
        if (rand < node.flux / mTotalFlux)
            return node;
        rand -= node.flux / mTotalFlux;
    }
    return mNodes.back();
}

float MSTree::pdf(const Node &node) const {
    return node.flux / mTotalFlux;
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

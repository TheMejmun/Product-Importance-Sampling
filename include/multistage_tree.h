//
// Created by Saman on 29.12.25.
//

#ifndef PIS_MULTISTAGE_TREE_H
#define PIS_MULTISTAGE_TREE_H

#include <random>
#include <vector>

#include "coords.h"

// Müller mentions 0.01f for a quadtree,
// We will use 0.1f for a 1D toy example
#define SUBDIV_THRESHOLD 0.1f

struct Node {
    float flux;
    float leftBoundary;
    float rightBoundary;

    bool operator <(const Node &rhs) const {
        return this->leftBoundary < rhs.leftBoundary;
    }

    [[nodiscard]] float width() const {
        return rightBoundary - leftBoundary;
    }
};

struct LightSample {
    float flux;
    float position;

    bool operator <(const LightSample &rhs) const {
        return this->position < rhs.position;
    }
};

struct MSTSample {
    Polar wo;
    float pdf;
};

// TODO convert this to a binary tree
class MSTree {
public:
    MSTree() = default;

    // Deposit a sample into the tree
    void add(float position, float value);

    // Reconstruct the tree from all collected samples
    void compile();

    // Get a random sample
    [[nodiscard]] const MSTSample &sample() const;

    // Calculate the probability for a given sample
    [[nodiscard]] float pdf(const Node &node) const;

    void exportToCsv(const std::string &filename);

private:
    float mTotalFlux = 0.f;
    std::vector<Node> mNodes;
    std::vector<LightSample> mLightSamples;
    std::discrete_distribution<size_t> mDistribution;
    std::vector<float> pdfs;

    void compileRec(float leftBoundary, float rightBoundary);
};

#endif //PIS_MULTISTAGE_TREE_H

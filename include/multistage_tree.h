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
    double flux;
    double startBoundary;
    double endBoundary;

    bool operator <(const Node &rhs) const {
        return this->startBoundary < rhs.startBoundary;
    }

    [[nodiscard]] double width() const {
        return endBoundary - startBoundary;
    }
};

struct LightSample {
    double flux;
    double position;

    bool operator <(const LightSample &rhs) const {
        return this->position < rhs.position;
    }
};

struct MSTSample {
    Polar wo;
    double pdf;
};

// TODO convert this to a binary tree
class MSTree {
public:
    MSTree() = default;

    // Deposit a sample into the tree
    void add(double position, double value);

    // Reconstruct the tree from all collected samples
    void compile();

    // Get a random sample
    [[nodiscard]] MSTSample sample() const;

    // Calculate the probability for a given sample
    [[nodiscard]] double pdf(const Node &node) const;

    void exportToCsv(const std::string &filename);

    std::vector<Node> &getNodes();

    std::vector<double> &getPdfs();

    const std::vector<Node> &getNodes() const;

    const std::vector<double> &getPdfs() const;

    MSTree copy() const;

private:
    double mTotalFlux = 0.0;
    std::vector<Node> mNodes;
    std::vector<LightSample> mLightSamples;
    std::vector<double> mPdfs;

    void compileRec(double leftBoundary, double rightBoundary);
};

#endif //PIS_MULTISTAGE_TREE_H

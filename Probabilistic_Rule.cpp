#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <limits>
#include <utility>
#include <chrono>

// Define types for clarity
using Arc = std::pair<int, int>;
using Graph = std::vector<std::vector<double>>;

struct TransitionRuleParameters {
    double alpha;  // Importance of pheromone
    double beta;   // Importance of heuristic information
    double q0;     // Exploitation threshold
};

// Function to compute the k2 score (stub for demonstration purposes)
double k2Score(int node, const std::vector<int>& parentSet) {
    // Placeholder function for calculating the k2 score.
    // Replace this with the actual k2 score computation logic.
    return static_cast<double>(node + parentSet.size());
}

// Function to compute heuristic information (gij)
double computeHeuristicInformation(int parent, int child, const std::vector<int>& parentSet, const std::vector<int>& nodes) {
    // Compute f(Xi; P(Xi) \cup {Xj})
    std::vector<int> newParentSet = parentSet;
    newParentSet.push_back(parent);
    double scoreWithParent = k2Score(child, newParentSet);

    // Compute f(Xi; P(Xi))
    double scoreWithoutParent = k2Score(child, parentSet);

    // Return gij
    return scoreWithParent - scoreWithoutParent;
}

// Function to compute the probabilistic transition rule
Arc probabilisticTransitionRule(const Graph& pheromoneLevels,
                                const Graph& heuristicInfo,
                                const std::vector<Arc>& candidateArcs,
                                const TransitionRuleParameters& params) {
    // Initialize random number generator
    // std::random_device rd;
    // std::mt19937 gen(rd());
    std::mt19937 gen(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));

    std::uniform_real_distribution<> dis(0.0, 1.0);

    // Generate a random number q
    double q = dis(gen);

    Arc selectedArc;

    if (q <= params.q0) {
        // Exploitation: Select the arc with the maximum value of [s_rl^alpha * g_rl^beta]
        double maxValue = -std::numeric_limits<double>::infinity();
        for (const auto& arc : candidateArcs) {
            int r = arc.first; // Parent
            int l = arc.second; // Child
            double value = std::pow(pheromoneLevels[r][l], params.alpha) *
                           std::pow(heuristicInfo[r][l], params.beta);
            if (value > maxValue) {
                maxValue = value;
                selectedArc = arc;
            }
        }
    } else {
        // Exploration: Select an arc probabilistically based on p_ij
        std::vector<double> probabilities;
        double totalProbability = 0.0;

        for (const auto& arc : candidateArcs) {
            int r = arc.first; // Parent
            int l = arc.second; // Child
            double probability = std::pow(pheromoneLevels[r][l], params.alpha) *
                                 std::pow(heuristicInfo[r][l], params.beta);
            probabilities.push_back(probability);
            totalProbability += probability;
        }

        // Normalize probabilities
        for (auto& prob : probabilities) {
            prob /= totalProbability;
        }

        // Perform roulette wheel selection
        std::discrete_distribution<> roulette(probabilities.begin(), probabilities.end());
        int selectedIndex = roulette(gen);
        selectedArc = candidateArcs[selectedIndex];
    }

    return selectedArc;
}

int main() {
    // Example pheromone levels (s_rl) and heuristic information (g_rl)
    Graph pheromoneLevels = {
        {0.1, 0.2, 0.3},
        {0.2, 0.3, 0.4},
        {0.3, 0.4, 0.5}
    };

    Graph heuristicInfo = {
        {1.0, 2.0, 0.37},
        {1.8, 2.3, 2.8},
        {0.25, 1.3, 2.0}
    };

    // Candidate arcs (r, l) with constraints satisfied
    std::vector<Arc> candidateArcs = {{0, 1}, {1, 2}, {2, 0}};

    // Example parent sets for each node
    std::vector<std::vector<int>> parentSets = {
        {2}, // Parents of node 0
        {0}, // Parents of node 1
        {1}  // Parents of node 2
    };

    // Calculate heuristic information dynamically
    for (const auto& arc : candidateArcs) {
        int parent = arc.first;
        int child = arc.second;
        heuristicInfo[parent][child] = computeHeuristicInformation(parent, child, parentSets[child], {0, 1, 2});
    }

    // Parameters for the transition rule
    TransitionRuleParameters params = {1.0, 0.3, 0.7};
    for (int i=0; i<100; i++) {
        // Compute the next arc
        Arc nextArc = probabilisticTransitionRule(pheromoneLevels, heuristicInfo, candidateArcs, params);

        std::cout << "Selected Arc: (" << nextArc.first << ", " << nextArc.second << ")\n";
    }

    return 0;
}

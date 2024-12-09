#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>

using namespace std;

class DAG {
private:
    unordered_map<int, vector<int>> adjList; // DAG structure

public:
    void addEdge(int u, int v) {
        adjList[u].push_back(v);
    }

    vector<int> getParents(int node) {
        vector<int> parents;
        for (auto& pair : adjList) {
            if (find(pair.second.begin(), pair.second.end(), node) != pair.second.end()) {
                parents.push_back(pair.first);
            }
        }
        return parents;
    }

    double calculateK2Score(const vector<vector<int>>& dataset, int numValues) {
        double k2Score = 0.0;

        for (const auto& pair : adjList) {
            int node = pair.first;
            vector<int> parents = getParents(node);

            int q_i = pow(numValues, parents.size()); // Number of parent configurations
            vector<int> N_ij(q_i, 0);
            vector<vector<int>> N_ijk(q_i, vector<int>(numValues, 0));

            for (const auto& record : dataset) {
                int parentConfig = 0;
                for (int i = 0; i < parents.size(); ++i) {
                    parentConfig = parentConfig * numValues + record[parents[i]];
                }
                N_ij[parentConfig]++;
                N_ijk[parentConfig][record[node]]++;
            }

            for (int j = 0; j < q_i; ++j) {
                double term1 = tgamma(numValues) / tgamma(N_ij[j] + numValues);
                double term2 = 1.0;
                for (int k = 0; k < numValues; ++k) {
                    term2 *= tgamma(N_ijk[j][k] + 1);
                }
                term2 /= tgamma(N_ij[j] + 1);
                k2Score += log(term1 * term2);
            }
        }
        return k2Score;
    }
};

int main() {
    DAG dag;

    dag.addEdge(0, 2);
    dag.addEdge(1, 2);
    dag.addEdge(1, 3);

    vector<vector<int>> dataset = {
        {0, 0, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 1, 0},
        {1, 1, 0, 1}
    };


    int numValues = 2;

    double k2Score = dag.calculateK2Score(dataset, numValues);

    cout << "K2 Score of the DAG: " << k2Score << endl;

    return 0;
}

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;


int integerPow(int base, int exp) {
    int result = 1;
    while (exp > 0) {
        if (exp % 2 == 1)  // If exp is odd, multiply the result by base
            result *= base;
        base *= base;       // Square the base
        exp /= 2;           // Divide exp by 2
    }
    return result;
}


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

    bool isCyclicUtil(int node, vector<bool>& visited, vector<bool>& recStack) {
        if (!visited[node]) {
            visited[node] = true;
            recStack[node] = true;

            for (int neighbor : adjList[node]) {
                if (!visited[neighbor] && isCyclicUtil(neighbor, visited, recStack))
                    return true;
                else if (recStack[neighbor])
                    return true;
            }
        }
        recStack[node] = false;
        return false;
    }

    bool isCyclic() {
        vector<bool> visited(adjList.size(), false);
        vector<bool> recStack(adjList.size(), false);

        for (auto& pair : adjList) {
            if (isCyclicUtil(pair.first, visited, recStack))
                return true;
        }
        return false;
    }

    double calculateK2Score(const vector<vector<int>>& dataset, int numValues) {
        double k2Score = 0.0;

        for (const auto& pair : adjList) {
            int node = pair.first;
            vector<int> parents = getParents(node);

            int q_i = 1;
            for (size_t i = 0; i < parents.size(); ++i) {
                q_i *= numValues;
                if (q_i > dataset.size()) {
                    cerr << "Error: Too many parent configurations. Reduce graph complexity." << endl;
                    return -1;
                }
            }

            vector<int> N_ij(q_i, 0);
            vector<vector<int>> N_ijk(q_i, vector<int>(numValues, 0));

            for (const auto& record : dataset) {
                if (record.size()!=0) {
                    int parentConfig = 0;
                    for (size_t i = 0; i < parents.size(); i++) {
                        parentConfig = parentConfig * numValues + record[parents[i]];
                    }
                    N_ij[parentConfig]++;
                    N_ijk[parentConfig][record[node]]++;
                }
            }

            for (int j = 0; j < q_i; ++j) {
                double term1 = lgamma(numValues) - lgamma(N_ij[j] + numValues);
                double term2 = 0.0;
                for (int k = 0; k < numValues; ++k) {
                    term2 += lgamma(N_ijk[j][k] + 1);
                }
                term2 -= lgamma(N_ij[j] + 1);
                k2Score += term1 + term2;
            }
        }
        return k2Score;
    }
};

// Function to split a string by a delimiter
vector<string> split(const string& line, char delimiter) {
    vector<string> tokens;
    stringstream ss(line);
    string token;

    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// Function to read data from a CSV file
vector<vector<int>> read_data(string filePath) {
    ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "Failed to open the file: " << filePath << endl;
        exit(EXIT_FAILURE); // Exit the program if the file cannot be opened
    }

    vector<vector<int>> data;
    string line;

    while (getline(file, line)) {
        vector<string> row = split(line, ','); // Split the line into tokens
        // Transform "yes" to "1" and "no" to "0"
        vector<int> aux_row;
        for (const string& cell : row) {
            if (cell == "\"yes\"") {
                aux_row.push_back(1);
            } else if (cell == "\"no\"") {
                aux_row.push_back(0);
            }
        }
        data.push_back(aux_row);
    }

    file.close();
    return data;
}

int main() {
    DAG dag;

    // dag.addEdge(2, 0);
    // dag.addEdge(5, 2);
    // dag.addEdge(3, 1);
    // dag.addEdge(4, 1);
    // dag.addEdge(5, 3);
    // dag.addEdge(7, 4);
    // dag.addEdge(7, 5);
    // dag.addEdge(6, 5);
    // Adding edges to the DAG
    dag.addEdge(1, 2);
    dag.addEdge(2, 5);
    dag.addEdge(1, 3);
    dag.addEdge(1, 4);
    dag.addEdge(3, 5);
    dag.addEdge(4, 7);
    dag.addEdge(5, 7);
    dag.addEdge(1, 4);
    dag.addEdge(1, 7);

    // Check if the DAG is valid
    if (dag.isCyclic()) {
        cerr << "Error: The graph contains a cycle. Ensure it is a valid DAG." << endl;
        return EXIT_FAILURE;
    }
    // Reading dataset from the CSV file
    string filePath = "asia.csv";
    vector<vector<int>> data = read_data(filePath);

    int numValues = 2; // Assuming binary variables

    double k2Score = dag.calculateK2Score(data, numValues);
    cout << "K2 Score of the DAG: " << k2Score << endl;
    return 0;
}

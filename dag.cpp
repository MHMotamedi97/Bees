#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm> 
#include <cmath>
#include <set>



using namespace std;

class DAG {
    private:
        unordered_map<int, vector<int>> adjList;

        bool isCyclicUtil(int node, unordered_set<int>& visited, unordered_set<int>& recStack) {
            visited.insert(node);
            recStack.insert(node);

            for (int neighbor : adjList[node]) {
                if (recStack.count(neighbor)) return true; 
                if (!visited.count(neighbor) && isCyclicUtil(neighbor, visited, recStack)) return true;
            }

            recStack.erase(node);
            return false;
        }

        bool createsCycle(int u, int v) {
            adjList[u].push_back(v);

            unordered_set<int> visited, recStack;
            for (auto& pair : adjList) {
                if (!visited.count(pair.first) && isCyclicUtil(pair.first, visited, recStack)) {
                    adjList[u].pop_back();
                    return true;
                }
            }

            adjList[u].pop_back();
            return false;
        }

    public:

        set<int> extractUnieque() {
            set<int> uniqueIntegers;
            for (const auto& pair: adjList)
            {
                uniqueIntegers.insert(pair.first);
                for (int neighbor: pair.second) {
                    uniqueIntegers.insert(neighbor);
                }
            }
            for (auto node: uniqueIntegers) {
                cout << node << endl;
            }
            return uniqueIntegers;
        }

        bool addEdge(int u, int v) {
            if (createsCycle(u, v)) {
                cout << "Edge " << u << " -> " << v << " creates a cycle. Not added.\n";
                return false;
            }

            adjList[u].push_back(v);
            cout << "Edge " << u << " -> " << v << " added.\n";
            return true;
        }

        bool removeEdge(int u, int v) {
            auto& neighbors = adjList[u];
            auto it = find(neighbors.begin(), neighbors.end(), v);
            if (it != neighbors.end()) {
                neighbors.erase(it);
                cout << "Edge " << u << " -> " << v << " removed.\n";
                return true;
            }
            return false;
        }

        bool reverseEdge(int u, int v) {
            // Remove the edge u -> v
            if (removeEdge(u, v)) {
                // Add the edge v -> u if it doesn't create a cycle
                if (addEdge(v, u)) {
                    return true;
                } else {
                    // If reversing the edge creates a cycle, restore the original direction
                    addEdge(u, v);
                    return false;
                }
            }
            return false;
        }

        bool moveEdge(int u, int v) {
            // Get all parents of u and v
            vector<int> parentsU = getParents(u);
            vector<int> parentsV = getParents(v);

            // Ensure both nodes have at least one parent and they don't share a common parent
            if (parentsU.empty() || parentsV.empty()) {
                return false;
            }

            bool operationSuccessful = false;

            // Try moving edge for each combination of parentU and parentV
            for (int parentU : parentsU) {
                for (int parentV : parentsV) {
                    if (parentU == parentV) continue; // Skip if they share the same parent

                    // Remove the existing parent-child relationships
                    removeEdge(parentU, u);
                    removeEdge(parentV, v);

                    // Check for cycles after swapping the parents
                    if (addEdge(parentU, v) && addEdge(parentV, u)) {
                        operationSuccessful = true;
                        break; // Success, exit the inner loop
                    } else {
                        // Restore the original edges if the operation fails
                        removeEdge(parentU, v);
                        removeEdge(parentV, u);
                        addEdge(parentU, u);
                        addEdge(parentV, v);
                    }
                }
                if (operationSuccessful) break; // Exit the outer loop if successful
            }

            return operationSuccessful;
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
                    // term2 -= lgamma(N_ij[j] + 1);
                    k2Score += term1 + term2;
                }
            }
            return k2Score;
        }

        void display() {
            cout << "Graph Adjacency List:\n";
            for (auto& pair : adjList) {
                cout << pair.first << " -> ";
                for (int neighbor : pair.second) {
                    cout << neighbor << " ";
                }
                cout << endl;
            }
        }
};


vector<DAG> createRandomDAGs(int numDAGs, int numNodes, int numEdges) {
    vector<DAG> dags(numDAGs);

    srand(time(0)); // Seed for random number generation

    for (int i = 0; i < numDAGs; ++i) {
        cout << "\nCreating Random DAG " << i + 1 << ":\n";
        int edgesAdded = 0;
        while (edgesAdded < numEdges) {
            int u = rand() % numNodes;  // Random node u
            int v = rand() % numNodes;  // Random node v

            if (u != v && dags[i].addEdge(u, v)) { // Don't add self-loops and check cycle
                // cout << "Edge " << u << " -> " << v << " added.\n";
                edgesAdded++;
            }
        }
    }

    return dags;
}



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
    dag.addEdge(0, 2);
    dag.addEdge(2, 5);
    dag.addEdge(1, 3);
    dag.addEdge(1, 4);
    dag.addEdge(3, 5);
    dag.addEdge(4, 7);
    dag.addEdge(5, 7);
    dag.addEdge(5, 6);
    // vector<DAG> dags = createRandomDAGs(2, 8, 5);
    // for (auto& dag: dags) {
    //     dag.display();
    // }
        // Reading dataset from the CSV file
    string filePath = "asia.csv";
    vector<vector<int>> data = read_data(filePath);

    int numValues = 2; // Assuming binary variables

    double k2Score = dag.calculateK2Score(data, numValues);
    cout << "K2 Score of the DAG: " << k2Score << endl;
    return 0;
}

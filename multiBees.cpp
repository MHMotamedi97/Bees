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
#include <random>
#include <limits>
#include <utility>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;
// Define types for clarity
using Arc = pair<int, int>;
using Graph = vector<vector<double>>;

struct TransitionRuleParameters {
    double alpha;  // Importance of pheromone
    double beta;   // Importance of heuristic information
    double q0;     // Exploitation threshold
};

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
            // Check if the edge already exists
            auto& neighbors = adjList[u];
            if (find(neighbors.begin(), neighbors.end(), v) != neighbors.end()) {
                cout << "Edge " << u << " -> " << v << " already exists. Not added.\n";
                return false;
            }

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

        vector<Arc> getEdges() {
            vector<Arc> edges;
            for (auto& pair : adjList) {
                for (int neighbor : pair.second) {
                    edges.emplace_back(pair.first, neighbor);
                }
            }
            return edges;
        }

        int numNodes() {
            unordered_set<int> uniqueNodes;
            for (const auto& pair : adjList) {
                uniqueNodes.insert(pair.first);
                uniqueNodes.insert(pair.second.begin(), pair.second.end());
            }
            return uniqueNodes.size();
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




class Bee {
    private:
        vector<DAG> storedDAGs;
        vector<double> storedScores;

    public:
        Bee() {}

        DAG generateRandomDAG(int numNodes) {
            DAG randomDAG;
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dist(0, numNodes - 1);
            int maxEdges = numNodes * (numNodes - 1) / 2; // Maximum number of edges in a directed acyclic graph
            uniform_int_distribution<> edge_dist(0, maxEdges - 1);
            maxEdges = edge_dist(gen);
            for (int i = 0; i < maxEdges; ++i) {
                int parent = dist(gen);
                int child = dist(gen);
                if (parent != child) {
                    randomDAG.addEdge(parent, child);
                }
            }
            return randomDAG;
        }

        void performOperations(DAG& dag, const vector<vector<int>>& data, int numValues) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> nodeDist(0, dag.numNodes() - 1);

            // Keep original DAG
            storedDAGs.push_back(dag);
            double score = dag.calculateK2Score(data, numValues);
            storedScores.push_back(score);

            // Randomly add an edge
            DAG modifiedDAG = dag;
            int parent = nodeDist(gen);
            int child = nodeDist(gen);
            if (parent != child) {
                modifiedDAG.addEdge(parent, child);
            }
            storedDAGs.push_back(modifiedDAG);
            storedScores.push_back(modifiedDAG.calculateK2Score(data, numValues));

            // Randomly remove an edge
            modifiedDAG = dag;
            auto edges = modifiedDAG.getEdges();
            if (!edges.empty()) {
                uniform_int_distribution<> edgeDist(0, edges.size() - 1);
                auto edge = edges[edgeDist(gen)];
                modifiedDAG.removeEdge(edge.first, edge.second);
            }
            storedDAGs.push_back(modifiedDAG);
            storedScores.push_back(modifiedDAG.calculateK2Score(data, numValues));

            // Randomly reverse an edge
            modifiedDAG = dag;
            edges = modifiedDAG.getEdges();
            if (!edges.empty()) {
                uniform_int_distribution<> edgeDist(0, edges.size() - 1);
                auto edge = edges[edgeDist(gen)];
                modifiedDAG.reverseEdge(edge.first, edge.second);
            }
            storedDAGs.push_back(modifiedDAG);
            storedScores.push_back(modifiedDAG.calculateK2Score(data, numValues));

            // Randomly move an edge
            modifiedDAG = dag;
            edges = modifiedDAG.getEdges();
            if (!edges.empty()) {
                uniform_int_distribution<> edgeDist(0, edges.size() - 1);
                auto edge = edges[edgeDist(gen)];
                int newParent = nodeDist(gen);
                int newChild = nodeDist(gen);
                if (newParent != newChild) {
                    modifiedDAG.removeEdge(edge.first, edge.second);
                    modifiedDAG.addEdge(newParent, newChild);
                }
            }
            storedDAGs.push_back(modifiedDAG);
            storedScores.push_back(modifiedDAG.calculateK2Score(data, numValues));
        }

        const vector<DAG>& getStoredDAGs() const {
            return storedDAGs;
        }

        const vector<double>& getStoredScores() const {
            return storedScores;
        }
        DAG getBestDAG() const {
            if (storedDAGs.empty()) {
                throw runtime_error("No DAGs stored.");
            }

            auto maxIt = max_element(storedScores.begin(), storedScores.end());
            int index = distance(storedScores.begin(), maxIt);
            return storedDAGs[index];
        }
};



mutex resultMutex; // Mutex to protect shared data

void runBeeTask(Bee& bee, const vector<vector<int>>& data, int numValues, vector<DAG>& globalDAGs, vector<double>& globalScores) {
    DAG randomDAG = bee.generateRandomDAG(8); // Generate a random DAG with 8 nodes
    bee.performOperations(randomDAG, data, numValues);

    // Store results in a thread-safe manner
    lock_guard<mutex> guard(resultMutex);
    const auto& storedDAGs = bee.getStoredDAGs();
    const auto& storedScores = bee.getStoredScores();
    globalDAGs.insert(globalDAGs.end(), storedDAGs.begin(), storedDAGs.end());
    globalScores.insert(globalScores.end(), storedScores.begin(), storedScores.end());
}

int main() {
    // Example DAG and dataset setup
    string filePath = "asia.csv";
    vector<vector<int>> data = read_data(filePath); // Load dataset
    int numValues = 2; // Assuming binary variables

    int numBees = 2; // Number of employed bees (threads)
    vector<thread> threads;
    vector<DAG> globalDAGs;
    vector<double> globalScores;

    for (int i = 0; i < numBees; ++i) {
        Bee bee;
        threads.emplace_back(runBeeTask, ref(bee), cref(data), numValues, ref(globalDAGs), ref(globalScores));
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Find the best DAG from all bees
    auto maxIt = max_element(globalScores.begin(), globalScores.end());
    if (maxIt != globalScores.end()) {
        int index = distance(globalScores.begin(), maxIt);
        cout << "Best DAG score: " << *maxIt << endl;
        globalDAGs[index].display();
    } else {
        cout << "No DAGs generated." << endl;
    }

    return 0;
}

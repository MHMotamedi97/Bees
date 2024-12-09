#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm> 
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
        // Find parents of u and v (nodes that have u and v as children)
        int parentU = -1, parentV = -1;
        bool foundParentU = false, foundParentV = false;

        // Search for parent of u
        for (auto& pair : adjList) {
            if (find(pair.second.begin(), pair.second.end(), u) != pair.second.end()) {
                parentU = pair.first;
                foundParentU = true;
                break;
            }
        }

        // Search for parent of v
        for (auto& pair : adjList) {
            if (find(pair.second.begin(), pair.second.end(), v) != pair.second.end()) {
                parentV = pair.first;
                foundParentV = true;
                break;
            }
        }

        // If no parent is found for either node, return false
        if (!foundParentU || !foundParentV || parentU == parentV) return false;

        // Remove the existing parent-child relationships
        removeEdge(parentU, u);
        removeEdge(parentV, v);

        // Check for cycles after swapping the parents
        if (createsCycle(parentU, v) || createsCycle(parentV, u)) {
            // If adding the swapped edges creates a cycle, restore the original edges
            addEdge(parentU, u);
            addEdge(parentV, v);
            return false;
        }

        // Add the swapped edges
        addEdge(parentU, v);
        addEdge(parentV, u);
        return true;
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


int main() {
    DAG dag;

    dag.addEdge(1, 2);
    dag.addEdge(1, 3);
    dag.addEdge(2, 4);
    dag.addEdge(3, 4);

    dag.addEdge(4, 1);
    dag.removeEdge(3, 4);
    dag.display();
    dag.moveEdge(2, 4);
    dag.display();
    // vector<DAG> dags = createRandomDAGs(2, 8, 5);
    // for (auto& dag: dags) {
    //     dag.display();
    // }
    return 0;
}

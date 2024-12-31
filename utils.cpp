bool moveEdge(int u, int v) {
    // Get all parents of u and v
    vector<int> parentsU = getParents(u);
    vector<int> parentsV = getParents(v);

    // Ensure both nodes have at least one parent and they don't share a common parent
    if (parentsU.empty() || parentsV.empty() || any_of(parentsU.begin(), parentsU.end(), [&](int pu) {
            return find(parentsV.begin(), parentsV.end(), pu) != parentsV.end();
        })) {
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

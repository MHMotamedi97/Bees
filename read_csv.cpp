#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;

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

vector<vector<int>> read_data(string filePath = "asia.csv") {
    // Open the file
    ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "Failed to open the file: " << filePath << endl;
    }

    vector<vector<int>> data;

    // Read the CSV file line by line
    string line;
    while (getline(file, line)) {
        vector<string> row = split(line, ','); // Split the line into tokens
        // Transform "yes" to "1" and "no" to "0" in-place
        for (string& cell : row) {
            if (cell=="\"yes\"") {
                cell="1";
            }
            else if (cell == "\"no\"") {
                cell="0";
            }
        }
        row.erase(row.begin());
        vector<int> aux_row;
        for (string& cell : row) {
            if (cell=="0") {
                aux_row.push_back(0);
            }
            else if (cell=="1") {
                aux_row.push_back(1);
            }
        } 
        data.push_back(aux_row);
        }

    file.close();

    return data;
}

int main() {
    vector<vector<int>> data = read_data();
    // Print the content of the CSV file
    cout << "CSV Content:" << endl;
    for (const auto& row : data) {
        for (const auto& cell : row) {
            cout << cell << " ";
        }
        cout << endl;
    }
    return 0;
}
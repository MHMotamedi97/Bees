#include <boost/process.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>

namespace bp = boost::process;
using namespace std;

// Simulates a Bee's task
void beeTask(const string& dataFilePath, int numValues, const string& outputPath, int seed) {
    srand(seed);
    double score = rand() % 1000;
    string dag = "DAG_" + to_string(seed);

    ofstream outFile(outputPath);
    if (outFile.is_open()) {
        outFile << score << " " << dag << endl;
        outFile.close();
    }
}

int main() {
    string dataFilePath = "asia.csv";
    int numValues = 2;
    int numProcesses = 4;

    vector<thread> threads;
    vector<string> outputFiles;

    // Launch threads to handle tasks
    for (int i = 0; i < numProcesses; ++i) {
        string outputPath = "output_" + to_string(i) + ".txt";
        outputFiles.push_back(outputPath);

        threads.emplace_back([=]() {
            beeTask(dataFilePath, numValues, outputPath, time(0) + i);
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Aggregate results
    double bestScore = -1e9;
    string bestDAG;
    for (const auto& file : outputFiles) {
        ifstream inFile(file);
        double score;
        string dag;
        while (inFile >> score >> dag) {
            if (score > bestScore) {
                bestScore = score;
                bestDAG = dag;
            }
        }
        inFile.close();
    }

    cout << "Best DAG: " << bestDAG << " with score: " << bestScore << endl;

    return 0;
}

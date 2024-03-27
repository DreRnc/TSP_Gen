#include <iostream>
#include <fstream>
#include "utils/argument_parser.hpp"
#include "utils/utimer.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    int num_workers;
    bool track_time, use_crossover, verbose;
    string data_path;

    parseArguments(argc, argv, num_workers, track_time, use_crossover, data_path, verbose);

    if (verbose) {
        cout << "Number of workers: " << num_workers << endl;
        cout << "Track time: " << (track_time ? "Yes" : "No") << endl;
        cout << "Use crossover: " << (use_crossover ? "Yes" : "No") << endl;
        cout << "Data path: " << data_path << endl;
    }

    {
        if (verbose) utimer t0("Time to write matrix: ");
        std::ifstream inputFile(data_path);

    std::string line;
    while (std::getline(inputFile, line)) { // Read each line from the file
        std::cout << line << std::endl; // Print the line to the console
    }

    inputFile.close(); /
    }   
    // construct the matrix

    // implement the genetica algorithm
    return 0;
}
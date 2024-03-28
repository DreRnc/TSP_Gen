#include <iostream>
#include <fstream>
#include <string>
#include "src/datafuncs.hpp"
#include "utils/argument_parser.hpp"
#include "utils/utimer.hpp"

using namespace std;
using FloatMatrix = vector<vector<float>>;

class GeneticAlgorithm (){
    //
}

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
        if (verbose) utimer t0("Time to generate the distance matrix: ");
        FloatMatrix distance_matrix = generate_distance_matrix(data_path);

    }   
    // construct the matrix

    // implement the genetic algorithm
    return 0;
}
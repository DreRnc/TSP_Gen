#ifndef ARGUMENT_PARSER_HPP
#define ARGUMENT_PARSER_HPP

#include <cstdlib>
#include <iostream>
#include <string>

void printHelp() {
    std::cout << "Usage: main [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -w <num_workers>             Set the number of workers" << std::endl;
    std::cout << "  -t                           Enable tracking of time" << std::endl;
    std::cout << "  -p <population_size>         Set the population size" << std::endl;
    std::cout << "  -g <num_generations>         Set the number of generations" << std::endl;
    std::cout << "  -p <num_parents>             Set the number of generations" << std::endl;
    std::cout << "  -d <data_path>               Set the data path (default: data/italy.tsp)" << std::endl;
    std::cout << "  -v                           Verbose: print extra information" << std::endl;
    std::cout << "  -h                           Print this help message" << std::endl;
}

void parseArguments(int argc,
                    char* argv[],
                    int& num_workers,
                    bool& track_time,
                    int& population_size,
                    int& num_generations,
                    int& num_parents,
                    std::string& data_path,
                    bool& verbose) {
    num_workers = 1;
    track_time = false;
    population_size = 100;
    num_generations = 10;
    num_parents = 10;
    data_path = "data/italy.tsp";
    verbose = false;

    if(num_parents > population_size){
        std::cerr << "Cannot have num_population > population_size" << endl;
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w" && i + 1 < argc) {
            num_workers = std::atoi(argv[++i]);
        } else if (arg == "-t") {
            track_time = true;
        } else if (arg == "-p" && i + 1 < argc) {
            population_size = std::atoi(argv[++i]);
        } else if (arg == "-g" && i + 1 < argc) {
            num_generations = std::atoi(argv[++i]);
        } else if (arg == "-p" && i + 1 < argc) {
            num_parents = std::atoi(argv[++i]);
        } else if (arg == "-d" && i + 1 < argc) {
            data_path = argv[++i];
        } else if (arg == "-v") {
            verbose = true;
        } else if (arg == "-h") {
            printHelp();
            exit(0);
        } else {
            std::cerr << "Error: Unknown option or missing argument: " << arg << std::endl;
            printHelp();
            exit(1);
        }
    }
}

#endif
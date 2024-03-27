#ifndef ARGUMENT_PARSER_HPP
#define ARGUMENT_PARSER_HPP

#include <cstdlib>
#include <iostream>
#include <string>

void printHelp() {
    std::cout << "Usage: main [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -w <num_workers>       Set the number of workers" << std::endl;
    std::cout << "  -t                      Enable tracking of time" << std::endl;
    std::cout << "  -c                      Enable crossover" << std::endl;
    std::cout << "  -d <data_path>         Set the data path (default: data/italy.tsp)" << std::endl;
    std::cout << "  -v                      Verbose: print extra information" << std::endl;
    std::cout << "  -h                      Print this help message" << std::endl;
}

void parseArguments(int argc, 
                    char* argv[], 
                    int& num_workers, 
                    bool& track_time, 
                    bool& use_crossover, 
                    std::string& data_path, 
                    bool& verbose) {
    num_workers = 1;
    track_time = false;
    use_crossover = false;
    data_path = "data/italy.tsp";
    verbose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w" && i + 1 < argc) {
            num_workers = std::atoi(argv[++i]);
        } else if (arg == "-t") {
            track_time = true;
        } else if (arg == "-c") {
            use_crossover = true;
        } else if (arg == "-v") {
            verbose = true;
        } else if (arg == "-d" && i + 1 < argc) {
            data_path = argv[++i];
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
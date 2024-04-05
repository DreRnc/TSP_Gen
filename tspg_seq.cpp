#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "src/distancefuncs.hpp"
#include "utils/argumentparser.hpp"
#include "utils/utimer.hpp"
#include "src/geneticfuncs.hpp"
#include "utils/genetictimer.hpp"

using namespace std;
using Matrix = vector<vector<double>>;

unsigned int seed = 42;
mt19937 gen(seed);

// If this slowes down, put it in the class and push into vector such as in dyn version
long selection_time, crossover_time, mutation_time, evaluation_time, merge_time, non_serial_time;

class TSPGenSeq {
public:
    TSPGenSeq(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), timer(timer) {
    }

    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }

    void evolve(vector<long>& times) {
        START(start)

        vector<Individual> parents = select_parents(population, num_parents, gen);
        STOP(start, selection_time)
        //timer.recordSelectionTime();

        vector<Individual> offspring = crossover_population(parents, gen);
        STOP(start, crossover_time)
        //timer.recordCrossoverTime();

        mutate(offspring, gen);
        STOP(start, mutation_time)
        //timer.recordMutationTime();

        evaluate_population(offspring, distance_matrix);
        STOP(start, evaluation_time)
        //timer.recordEvaluationTime();

        merge(population, offspring);
        STOP(start, merge_time)
        //timer.recordMergeTime();
        times.push_back(selection_time);
        
    }

    void run() {
        START(start_total)
        timer.reset();

        vector<long> times;

        for (int i = 0; i < num_generations; i++) {
            evolve(times);
            timer.recordSelectionTime(selection_time);
            timer.recordCrossoverTime(crossover_time);
            timer.recordMutationTime(mutation_time);
            timer.recordEvaluationTime(evaluation_time);
            timer.recordMergeTime(merge_time);
        }
        
        STOP(start_total, total_time)
        timer.recordTotalTime(total_time);
    }

    Individual get_best() {
        sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.fitness > b.fitness;
        });
        Individual best = population[0];
        return best;
    }

private:
    int route_length;
    const Matrix distance_matrix;
    int num_parents;
    int population_size;
    int num_generations;
    vector<Individual> population;
    GeneticTimer& timer;
};

int main(int argc, char* argv[]) {
    int num_workers, population_size, num_generations, num_parents;
    bool track_time, verbose;
    string data_path, file_path;
    bool parallel = false;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_generations, num_parents, data_path, file_path, verbose);

    num_workers = 1;

    if (verbose) {
    cout << "Number of workers: " << num_workers << endl;
    cout << "Track time: " << (track_time ? "Yes" : "No") << endl;
    cout << "Population size: " << population_size << endl;
    cout << "Number of generations: " << num_generations << endl;
    cout << "Data path: " << data_path << endl;
    }
    
    vector<City> cities = generate_city_vector(data_path);
    int route_length = cities.size();
    const Matrix distance_matrix = generate_distance_matrix(cities);
    
    GeneticTimer gentimer(parallel, num_generations);
    TSPGenSeq ga(route_length, distance_matrix, population_size, num_generations, num_parents, gentimer);

    START(start_init)
    ga.initialize();
    STOP(start_init, initialization_time)
    gentimer.recordInitializationTime(initialization_time);

    if(verbose) cout << "Best random route: " << ga.get_best().score << endl;

    ga.run();

    if(verbose) cout << "Best route after genetic alg: " << ga.get_best().score << endl;

    if(track_time) gentimer.writeTimesToFile(file_path, num_workers);

    return 0;
}
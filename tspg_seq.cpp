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

class TSPGenSeq {
public:
    TSPGenSeq(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), timer(timer) {
    }

    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }

    void evolve() {
        timer.start();

        vector<Individual> parents = select_parents(population, num_parents, gen);
        timer.recordSelectionTime();

        vector<Individual> offspring = crossover_population(parents, gen);
        timer.recordCrossoverTime();

        mutate(offspring, gen);
        timer.recordMutationTime();

        evaluate_population(offspring, distance_matrix);
        timer.recordEvaluationTime();

        merge(population, offspring, gen);
        timer.recordMergeTime();
    }

    void run() {
        timer.reset();
        timer.start_total();
        for (int i = 0; i < num_generations; i++) {
            cout << "Starting evolution: " << i << endl;
            evolve();
        }
        timer.recordTotalTime();
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
    string data_path;
    bool parallel = false;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_generations, num_parents, data_path, verbose);

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
    
    GeneticTimer gentimer(parallel);
    TSPGenSeq ga(route_length, distance_matrix, population_size, num_generations, num_parents, gentimer);

    gentimer.start();
    ga.initialize();
    gentimer.recordInitializationTime();

    if(verbose) cout << "Best random route: " << ga.get_best().score << endl;

    ga.run();

    if(verbose) cout << "Best route after genetic alg: " << ga.get_best().score << endl;

    if(track_time) gentimer.writeTimesToFile("results/Times.txt", num_workers);

    return 0;
}
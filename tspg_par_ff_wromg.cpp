#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include "src/distancefuncs.hpp"
#include "utils/argumentparser.hpp"
#include "utils/utimer.hpp"
#include "src/geneticfuncs.hpp"
#include "src/utilfuncs.hpp"
#include "utils/genetictimer.hpp"
#include "fastflow/ff/ff.hpp"
#include "fastflow/ff/poolEvolution.hpp"

using namespace std;
using namespace ff;
using Matrix = vector<vector<double>>;

unsigned int seed = 42;
mt19937 gen(seed);

class TSPGenParFF {
public:
    TSPGenParFF(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer, int num_workers)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), timer(timer), num_workers(num_workers) {
    }
    
    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }

    void run() {
        poolEvolution<Individual, TSPGenParFF> pool(num_workers, population, selection_ff, evolution_ff, filter_ff, termination_ff, this);
        pool.run_and_wait_end();
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
    int generations_passed;
    vector<Individual> population;
    vector<Individual> parents;
    vector<Individual> offspring;
    GeneticTimer& timer;
    int num_workers;
    vector<tuple<long,long,long,long>> load_balancing_stats;
    vector<long> time_loads;
    mutex m;

    void selection_ff(ParallelForReduce<Individual>& , vector<Individual>& population, vector<Individual>& selected_parents, TSPGenParFF& env) {
        selected_parents.clear();
        parents.clear();
        parents = select_parents(population, num_parents, gen);
        selected_parents.insert(selected_parents.end(), parents.begin(), parents.end());
    }

    const Individual& evolution_ff(Individual& individual, const TSPGenParFF& env, const int) {

        uniform_int_distribution<int> dist_parents(0, env.parents.size() - 1);
        int parent2 = dist_parents(gen);

        int size = individual.chr.size();

        uniform_int_distribution<int> dist_chr(0, size - 1);
        int cutpoint1 = dist_chr(gen);
        int cutpoint2 = dist_chr(gen);

        individual = _cross(individual, env.parents[parent2], cutpoint1, cutpoint2, size);
        
        int pos1 = dist_chr(gen);
        int pos2 = dist_chr(gen);

        swap(individual.chr[pos1], individual.chr[pos2]);

        return individual;
    }

    // Should be our merge
    void filter_ff(ParallelForReduce<Individual>&, vector<Individual>& population, vector<Individual>& offspring, TSPGenParFF& env) {
        evaluate_population(offspring, distance_matrix);
        population = merge_ff(population, offspring);
    }

    bool termination_ff(const vector<Individual> &population,  TSPGenParFF& env) {
        if (env.generations_passed < env.num_generations){
            env.generations_passed += 1;
            return false;
        return true;
        }
    }

    vector<Individual> merge_ff(vector<Individual>& population, vector<Individual>& offspring){
        merge(population, offspring, gen);
        return population;
    }
};

int main(int argc, char* argv[]) {
    int num_workers, population_size, num_generations, num_parents;
    bool track_time, verbose;
    string data_path, file_path;
    bool parallel = true;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_generations, num_parents, data_path, file_path, verbose);

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
    TSPGenParFF ga(route_length, distance_matrix, population_size, num_generations, num_parents, gentimer, num_workers);

    gentimer.start();
    ga.initialize();
    gentimer.recordInitializationTime();

    if(verbose) cout << "Best random route: " << ga.get_best().score << endl;

    ga.run();

    if(verbose) cout << "Best route after genetic alg: " << ga.get_best().score << endl;

    if(track_time) gentimer.writeTimesToFile(file_path, num_workers);

    return 0;
}
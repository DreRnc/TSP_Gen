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

struct TSPGenEnv {
    TSPGenEnv(int route_length, int num_gen, int num_parents, const Matrix& distance_matrix, GeneticTimer& gentimer) : 
            num_gen(num_gen), num_parents(num_parents), distance_matrix(distance_matrix), gentimer(gentimer) {}

    int num_gen;
    int num_parents;

    const Matrix distance_matrix;

    GeneticTimer& gentimer;

    vector<Individual> parents;

    int gen_passed = 0;
};

void selection(ParallelForReduce<Individual>& , vector<Individual>& population, vector<Individual>& selected_parents, TSPGenEnv& env) {
        selected_parents.clear();
        env.parents.clear();
        env.parents = select_parents(population, env.num_parents, gen);
        selected_parents.insert(selected_parents.end(), env.parents.begin(), env.parents.end());
}

const Individual& evolution(Individual& individual, const TSPGenEnv& env, const int) {

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

vector<Individual> _merge(vector<Individual>& population, vector<Individual>& offspring){
    merge(population, offspring, gen);
    return population;
}

void filter(ParallelForReduce<Individual>&, vector<Individual>& population, vector<Individual>& offspring, TSPGenEnv& env) {
    evaluate_population(offspring, env.distance_matrix);
    population = _merge(population, offspring);
}

bool termination(const vector<Individual> &population,  TSPGenEnv& env) {
    if (env.gen_passed < env.num_gen){
        env.gen_passed += 1;
        return false;
        }
    return true;
}

Individual get_best(vector<Individual>& population) {
    sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });
    Individual best = population[0];
    return best;
}

int main(int argc, char* argv[]) {
    int num_workers, population_size, num_gen, num_parents;
    bool track_time, verbose;
    string data_path, file_path;
    bool parallel = true;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_gen, num_parents, data_path, file_path, verbose);

    if (verbose) {
    cout << "Number of workers: " << num_workers << endl;
    cout << "Track time: " << (track_time ? "Yes" : "No") << endl;
    cout << "Population size: " << population_size << endl;
    cout << "Number of gen: " << num_gen << endl;
    cout << "Data path: " << data_path << endl;
    }
    
    vector<City> cities = generate_city_vector(data_path);
    int route_length = cities.size();
    const Matrix distance_matrix = generate_distance_matrix(cities);
    
    GeneticTimer gentimer(parallel);
    TSPGenEnv env(route_length, num_gen, num_parents, distance_matrix, gentimer);

    START(start);

    gentimer.start();
    vector<Individual> population = initialize_population(population_size, route_length, gen);
    evaluate_population(population, distance_matrix);
    gentimer.recordInitializationTime();

    if(verbose) cout << "Best random route: " << get_best(population).score << endl;

    poolEvolution<Individual, TSPGenEnv> pool(num_workers, population, selection, evolution, filter, termination, env);
	pool.run_and_wait_end();

    STOP(start, time);

    ofstream outfile(file_path, ios::app);

    outfile << "Time with " << num_workers << " workers: " << time << "\n" << endl;
    
    cout << "Time statistics of the run have been written to file " << file_path << " successfully." << endl;

    if(verbose) cout << "Best route after genetic alg: " << get_best(population).score << endl;

    //if(track_time) gentimer.writeTimesToFile(file_path, num_workers);

    return 0;
}
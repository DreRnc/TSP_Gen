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

using namespace std;
using Matrix = vector<vector<double>>;

unsigned int seed = 42;
mt19937 gen(seed);

class TSPGenParNative {
public:
    TSPGenParNative(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer, int num_workers)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), timer(timer), num_workers(num_workers) {
    }
    
    /*
    void initialize() {
        
        thread tids[num_workers];

        for(int i=0; i<num_workers; i++){
            tids[i] = thread([&, i](){ 
                
                int chunk_size = population_size / num_workers;
                // if I'm the last one, take what remains (chunk_size + population_size % num_workers)
                int size = (i == (num_workers-1) ? (population_size - chunk_size*i) : (chunk_size));

                vector<Individual> chunk_population;

                chunk_population = initialize_population(size, route_length, gen);
                evaluate_population(chunk_population, distance_matrix);

                // Lock when pushing in the global vector of population
                unique_lock chunk_lock(m);
                population.insert(population.end(), chunk_population.begin(), chunk_population.end());
            });
        }
        for(int i=0; i<num_workers; i++){
            tids[i].join();
        }

        if(population.size() != population_size) cout<<"Error in initialization!"<<endl;
    }
    
    */
    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }
    

    void evolve() {
        timer.start();

        offspring.clear();

        thread tids[num_workers];
        fill(time_loads.begin(), time_loads.end(), 0);

        for(int i=0; i<num_workers; i++){
            tids[i] = thread([&, i](){ evolve_chunk(i); });
        }
        for(int i=0; i<num_workers; i++){
            tids[i].join();
        }

        tuple<long, long, long, long> stats = vec_stats(time_loads);
        load_balancing_stats.push_back(stats);

        timer.recordOffspringParTime();

        merge(population, offspring, gen);
        timer.recordMergeTime();
    }

    void run() {
        time_loads.resize(num_workers);
        timer.reset();
        timer.start_total();
        for (int i = 0; i < num_generations; i++) {
            cout << "Starting evolution: " << i << endl;
            evolve();
        }
        timer.recordTotalTime();
        timer.recordLoadBalanceStats(load_balancing_stats);
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
    vector<Individual> offspring;
    GeneticTimer& timer;
    int num_workers;
    vector<tuple<long,long,long,long>> load_balancing_stats;
    vector<long> time_loads;
    mutex m;

    void evolve_chunk(int i){
        START(start);
        int chunk_size = num_parents / num_workers;
        int size = (i == (num_workers-1) ? (num_parents - chunk_size*i) : (chunk_size));

        vector<Individual> chunk_parents = select_parents(population, size, gen);

        vector<Individual> chunk_offspring = crossover_population(chunk_parents, gen);

        mutate(chunk_offspring, gen);

        evaluate_population(chunk_offspring, distance_matrix);

        // Lock when pushing in the global vector of offsrping and timings
        unique_lock chunk_lock(m);
        offspring.insert(offspring.end(), chunk_offspring.begin(), chunk_offspring.end());

        STOP(start, worker_time);
        time_loads[i] = worker_time;
    }

    void initialize_chunk(int i){
        int chunk_size = population_size / num_workers;
        // if I'm the last one, take what remains (chunk_size + population_size % num_workers)
        int size = (i == (num_workers-1) ? (population_size - chunk_size*i) : (chunk_size));

        vector<Individual> chunk_population;

        chunk_population = initialize_population(size, route_length, gen);
        evaluate_population(chunk_population, distance_matrix);

        // Lock when pushing in the global vector of population
        unique_lock chunk_lock(m);
        population.insert(population.end(), chunk_population.begin(), chunk_population.end());
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
    TSPGenParNative ga(route_length, distance_matrix, population_size, num_generations, num_parents, gentimer, num_workers);

    gentimer.start();
    ga.initialize();
    gentimer.recordInitializationTime();

    if(verbose) cout << "Best random route: " << ga.get_best().score << endl;

    ga.run();

    if(verbose) cout << "Best route after genetic alg: " << ga.get_best().score << endl;

    if(track_time) gentimer.writeTimesToFile(file_path, num_workers);

    return 0;
}
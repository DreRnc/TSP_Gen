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
            tids[i] = thread([&, i](){ initialize_chunk(i); });
        }
        for(int i=0; i<num_workers; i++){
            tids[i].join();
        }

        population = mergeChunks(init_chunks);

        if(population.size() != population_size) cout<<"Error in initialization!"<<endl;
    }
    */

    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }

    void evolve() {
        timer.start();

        vector<Individual> parents = select_parents(population, num_parents, gen);
        timer.recordSelectionTime();

        offspring_chunks.clear();

        thread tids[num_workers];
        fill(time_loads.begin(), time_loads.end(), 0);

        for(int i=0; i<num_workers; i++){
            tids[i] = thread([&, i](){ evolve_chunk(parents, i); });
        }
        for(int i=0; i<num_workers; i++){
            tids[i].join();
        }

        tuple<long, long, long, long> stats = vec_stats(time_loads);
        load_balancing_stats.push_back(stats);

        vector<Individual> offspring = mergeChunks(offspring_chunks);
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
    vector<vector<Individual>> offspring_chunks;
    vector<vector<Individual>> init_chunks;
    GeneticTimer& timer;
    int num_workers;
    vector<tuple<long,long,long,long>> load_balancing_stats;
    vector<long> time_loads;
    mutex m;

    void evolve_chunk(vector<Individual>& parents, int i){
        START(start);
        int delta = parents.size() / num_workers; 
        int from = i*delta;
        // if I'm the last one, take input.size(), else take (it1)*delta
        int to = (i == (num_workers-1) ? parents.size() : (i+1)*delta); 

        vector<Individual> chunk(parents.begin() + from, parents.begin() + to);

        vector<Individual> chunk_offspring = crossover_population(chunk, gen);
        mutate(chunk_offspring, gen);

        evaluate_population(chunk_offspring, distance_matrix);

        // Lock when pushing in the global vector of evolved chunks and timings
        unique_lock chunk_lock(m);
        offspring_chunks.push_back(chunk_offspring);
        STOP(start, worker_time);
        time_loads[i] = worker_time;
    }

    void initialize_chunk(int i){
        int chunk_size = population_size / num_workers;
        // if I'm the last one, take what remains (chunk_size + population_size % num_workers)
        int size = (i == (num_workers-1) ? (population_size - chunk_size*i) : (chunk_size));

        vector<Individual> population_chunk;

        population_chunk = initialize_population(size, route_length, gen);
        evaluate_population(population_chunk, distance_matrix);

        // Lock when pushing in the global vector of evolved chunks
        unique_lock chunk_lock(m);
        init_chunks.push_back(population_chunk);
    }

    vector<Individual> mergeChunks(const vector<vector<Individual>>& chunks) {
        vector<Individual> mergedChunks;
        
        // Reserve space for the merged vector to avoid frequent reallocations
        size_t totalSize = 0;
        for (const auto& vec : chunks) {
            totalSize += vec.size();
        }
        mergedChunks.reserve(totalSize);
        
        // Iterate over each vector and append its elements to the merged vector
        for (const auto& vec : chunks) {
            mergedChunks.insert(mergedChunks.end(), vec.begin(), vec.end());
        }
        
        return mergedChunks;
    }
    
};

int main(int argc, char* argv[]) {
    int num_workers, population_size, num_generations, num_parents;
    bool track_time, verbose;
    string data_path;
    bool parallel = true;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_generations, num_parents, data_path, verbose);

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

    if(track_time) gentimer.writeTimesToFile("results/Times.txt");

    return 0;
}
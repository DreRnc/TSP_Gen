#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <queue>
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

class DynamicChunks {
public:
    DynamicChunks() {}

    void enqueue(vector<Individual> chunk) {
        lock_guard<mutex> lock(queue_mutex);
        tasks.push(chunk);
    }

    vector<Individual> get_chunk() {
        lock_guard<mutex> lock(queue_mutex);
        if (tasks.empty()) {
            return {};
        }
        vector<Individual> chunk = tasks.front();
        tasks.pop();
        return chunk;
    }

private:
    queue<vector<Individual>> tasks;
    mutex queue_mutex;
};

    
class TSPGenParNativeDyn {
public:
    TSPGenParNativeDyn(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer, int num_workers, int dyn_chunk_size)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), 
        timer(timer), num_workers(num_workers), dyn_chunk_size(dyn_chunk_size){
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

        DynamicChunks task_pool;
        thread tids[num_workers];

        for (size_t i = 0; i < parents.size(); i += dyn_chunk_size) {
            vector<Individual>::iterator last = parents.begin() + min(i + dyn_chunk_size, parents.size());
            vector<Individual> chunk(parents.begin() + i, last);
            task_pool.enqueue(std::move(chunk));
        }

        for(int i=0; i<num_workers; i++){
            tids[i] = thread([&, i](){ 
                START(start)
                while (true) {
                    vector<Individual> chunk = task_pool.get_chunk();
                    if (chunk.empty()) break;
                    evolve_chunk(chunk);
                }
                STOP(start, worker_time);
                unique_lock chunk_lock(m);
                time_loads[i] = worker_time;
            });
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
    int dyn_chunk_size;
    vector<tuple<long,long,long,long>> load_balancing_stats;
    vector<long> time_loads;
    mutex m;

    void evolve_chunk(vector<Individual>& parents_chunk){

        vector<Individual> chunk_offspring = crossover_population(parents_chunk, gen);
        mutate(chunk_offspring, gen);

        evaluate_population(chunk_offspring, distance_matrix);

        // Lock when pushing in the global vector of evolved chunks and timings
        unique_lock chunk_lock(m);
        offspring_chunks.push_back(chunk_offspring);
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
    int num_workers, population_size, num_generations, num_parents, dyn_chunk_size;
    bool track_time, verbose;
    string data_path, file_path;
    bool parallel = true;

    parseArguments(argc, argv, num_workers, track_time, population_size, num_generations, num_parents, data_path, file_path, verbose);

    // Add a parsing argument for the size of dynamic chunks
    for (int i = 1; i < argc; ++i) { 
        std::string arg = argv[i];
        if (arg == "-ds") {
            dyn_chunk_size = std::atoi(argv[++i]);
        }
    }
    
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
    TSPGenParNativeDyn ga(route_length, distance_matrix, population_size, num_generations, num_parents, gentimer, num_workers, dyn_chunk_size);

    gentimer.start();
    ga.initialize();
    gentimer.recordInitializationTime();

    if(verbose) cout << "Best random route: " << ga.get_best().score << endl;

    ga.run();

    if(verbose) cout << "Best route after genetic alg: " << ga.get_best().score << endl;

    if(track_time) gentimer.writeTimesToFile(file_path, num_workers);

    return 0;
}
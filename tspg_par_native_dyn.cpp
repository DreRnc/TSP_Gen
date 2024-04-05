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

    void generate_queue (int num_parents, int chunk_size) {
        int num_chunks = num_parents / chunk_size;
        for(int i = 0; i < num_chunks; i++){
            if(i == num_chunks - 1) {
                tickets.push(chunk_size + num_parents % chunk_size);
            }
            else {
                tickets.push(chunk_size);
            }
        }
    }

    int get_ticket() {
        lock_guard<mutex> lock(queue_mutex);
        if (tickets.empty()) {
            return -1;
        }
        int chunk = tickets.front();
        tickets.pop();
        return chunk;
    }

private:
    queue<int> tickets;
    mutex queue_mutex;
};

    
class TSPGenParNativeDyn {
public:
    TSPGenParNativeDyn(int route_length, const Matrix& distance_matrix, int population_size, int num_generations, int num_parents, GeneticTimer& timer, int num_workers, int dyn_chunk_size)
        : route_length(route_length), distance_matrix(distance_matrix), population_size(population_size), num_generations(num_generations), num_parents(num_parents), 
        timer(timer), num_workers(num_workers), dyn_chunk_size(dyn_chunk_size){
    }

    void initialize() {
        population = initialize_population(population_size, route_length, gen);
        evaluate_population(population, distance_matrix);
    }

    void evolve() {
        timer.start();

        offspring.clear();

        DynamicChunks task_pool;
        task_pool.generate_queue(num_parents, dyn_chunk_size);

        thread tids[num_workers];
        fill(time_loads.begin(), time_loads.end(), 0);

        for(int i=0; i<num_workers; i++){
            tids[i] = thread([&, i](){ 
                START(start)
                while (true) {
                    int chunk_size = task_pool.get_ticket();
                    if (chunk_size == -1) break;
                    evolve_chunk(chunk_size);
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

        timer.recordOffspringParTime();

        merge(population, offspring);
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
    int dyn_chunk_size;
    vector<tuple<long,long,long,long>> load_balancing_stats;
    vector<long> time_loads;
    mutex m;

    void evolve_chunk(int size){
        vector<Individual> chunk_parents = select_parents(population, size, gen);

        vector<Individual> chunk_offspring = crossover_population(chunk_parents, gen);

        mutate(chunk_offspring, gen);

        evaluate_population(chunk_offspring, distance_matrix);

        // Lock when pushing in the global vector of offsrping and timings
        unique_lock chunk_lock(m);
        offspring.insert(offspring.end(), chunk_offspring.begin(), chunk_offspring.end());
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


    if(track_time){
        gentimer.writeTimesToFile(file_path, num_workers);
        ofstream outfile(file_path, ios::app);
        outfile << "Dynamic chunk size: " << dyn_chunk_size << endl;
    }
    
    return 0;
}
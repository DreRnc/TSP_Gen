#ifndef GENETICTIMER_HPP
#define GENETICTIMER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <tuple>

class GeneticTimer {
public:
    
    GeneticTimer(bool parallel) : parallel(parallel) {}

    void start_total() {
        start_total_time = std::chrono::steady_clock::now();
    }

    void start() {
        start_time = std::chrono::steady_clock::now();
    }

    void stop() {
        end_time = std::chrono::steady_clock::now();
    }

    void reset() {
        selection_time.clear();
        crossover_time.clear();
        mutation_time.clear();
        evaluation_time.clear();
        merge_time.clear();
    }

    void recordInitializationTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        initialization_time = elapsed_time;
    }
    void recordSelectionTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        selection_time.push_back(elapsed_time);
    }

    void recordCrossoverTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        crossover_time.push_back(elapsed_time);
    }

    void recordMutationTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        mutation_time.push_back(elapsed_time);
    }

    void recordEvaluationTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        evaluation_time.push_back(elapsed_time);
    }

    void recordMergeTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        merge_time.push_back(elapsed_time);
    }

    void recordOffspringParTime() {
        stop();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        offspringpar_time.push_back(elapsed_time);
    }

    void recordTotalTime(){
        stop();
        total_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_total_time).count();
    }

    void writeTimesToFile(const std::string& filename, int num_workers) {
        convertTimesTotalToPhase();
        calculateAveragePhaseTimes();

        std::ofstream outfile(filename, std::ios::app);

        if (!outfile.is_open()) {
            std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
            return;
        }

        outfile << "----- Recording run times -----" << std::endl;
        outfile << "Number of workers: " << num_workers << std::endl;
        outfile << "Initialization time: " << initialization_time << std::endl;
        if(!parallel){
            outfile << "Selection average time: " << average_times[0] << std::endl;
            outfile << "Crossover average time: " << average_times[1] << std::endl;
            outfile << "Mutation average time: " << average_times[2] << std::endl;
            outfile << "Evaluation average time: " << average_times[3] << std::endl;
            outfile << "Merge average time: " << average_times[4] << std::endl;
        }
        else{
            outfile << "Offspring average time: " << average_times[0] << std::endl;
            outfile << "Merge average time: " << average_times[1] << std::endl;
            outfile << "Load balancing:" << std::endl;
            outfile << "Min load time among workers (average across generations): " << average_times[2] << std::endl;
            outfile << "Max load time among workers (average across generations): " << average_times[3] << std::endl;
            outfile << "Mean of load times among workers (average across generations): " << average_times[4] << std::endl;
            outfile << "Std of load times among workers (average across generations): " << average_times[5] << std::endl;

        }
        outfile << "\nTotal time: " << total_time << '\n' << std::endl;

        std::cout << "Time statistics of the run have been written to file " << filename << " successfully." << std::endl;

        outfile.close();
    }

    void recordLoadBalanceStats(vector<tuple<long, long, long, long>> load_balancing){
        for (auto elem : load_balancing){
            long min, max, mean, std;
            tie(min, max, mean, std) = elem;
            min_loads.push_back(min);
            max_loads.push_back(max);
            mean_loads.push_back(mean);
            std_loads.push_back(std);
        }
    }


private:    

    bool parallel;

    long initialization_time;
    long total_time;

    std::vector<long> selection_time;
    std::vector<long> crossover_time;
    std::vector<long> mutation_time;
    std::vector<long> evaluation_time;
    std::vector<long> merge_time;
    std::vector<long> offspringpar_time;

    std::vector<long> average_times;
    std::vector<long> min_loads;
    std::vector<long> max_loads;
    std::vector<long> mean_loads;
    std::vector<long> std_loads;

    std::chrono::steady_clock::time_point start_total_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    // Use integer division as we are not interested in decimals and numbers are very big, 
    // doubles may not be able to represent them
    long calculateAverageTime(const std::vector<long>& time_vector) {
        if (time_vector.empty()) {
            return 0;
        }
        return std::accumulate(time_vector.begin(), time_vector.end(), 0L) / (long)time_vector.size();
    }

    void calculateAveragePhaseTimes() {
        if (!parallel){
            average_times.push_back(calculateAverageTime(selection_time));
            average_times.push_back(calculateAverageTime(crossover_time));
            average_times.push_back(calculateAverageTime(mutation_time));
            average_times.push_back(calculateAverageTime(evaluation_time));
            average_times.push_back(calculateAverageTime(merge_time));
        }
        else {
            average_times.push_back(calculateAverageTime(offspringpar_time));
            average_times.push_back(calculateAverageTime(merge_time));
            average_times.push_back(calculateAverageTime(min_loads));
            average_times.push_back(calculateAverageTime(max_loads));
            average_times.push_back(calculateAverageTime(mean_loads));
            average_times.push_back(calculateAverageTime(std_loads));
        }
    }

    void convertTimesTotalToPhase(){
        int num_generations = merge_time.size();

        for(int i = 0; i < num_generations; i++){
            if (!parallel){
                merge_time[i] = merge_time[i] - evaluation_time[i];
                evaluation_time[i] = evaluation_time[i] - mutation_time[i];
                mutation_time[i] = mutation_time[i] - crossover_time[i];
                crossover_time[i] = crossover_time[i] - selection_time[i];
            }
            else {
                merge_time[i] = merge_time[i] - offspringpar_time[i];
            }
        }
    }
};


#endif
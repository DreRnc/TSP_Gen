#ifndef GENETICTIMER_HPP
#define GENETICTIMER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>

class GeneticTimer {
public:

    long initialization_time;
    
    GeneticTimer() {}

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

    std::vector<double> calculateAverageTimes() const {
        std::vector<double> average_times;
        average_times.push_back(calculateAverageTime(selection_time));
        average_times.push_back(calculateAverageTime(crossover_time));
        average_times.push_back(calculateAverageTime(mutation_time));
        average_times.push_back(calculateAverageTime(evaluation_time));
        average_times.push_back(calculateAverageTime(merge_time));
        return average_times;
    }

private:
    std::vector<long> selection_time;
    std::vector<long> crossover_time;
    std::vector<long> mutation_time;
    std::vector<long> evaluation_time;
    std::vector<long> merge_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    double calculateAverageTime(const std::vector<long>& time_vector) const {
        if (time_vector.empty()) {
            return 0.0;
        }
        return static_cast<double>(std::accumulate(time_vector.begin(), time_vector.end(), 0)) / time_vector.size();
    }
};

#endif
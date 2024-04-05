#ifndef GENETICTIMER_HPP
#define GENETICTIMER_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <numeric>
#include <tuple>

using namespace std;

class GeneticTimer {
public:
    
    GeneticTimer(bool parallel, int num_generations) : parallel(parallel), num_generations(num_generations) {}

    void reset() {
        selection_time.clear();
        crossover_time.clear();
        mutation_time.clear();
        evaluation_time.clear();
        merge_time.clear();
    }

    void recordInitializationTime(long elapsed_time) {
    initialization_time = elapsed_time;
    }

    void recordSelectionTime(long elapsed_time) {
        selection_time.push_back(elapsed_time);
    }

    void recordCrossoverTime(long elapsed_time) {
        crossover_time.push_back(elapsed_time);
    }

    void recordMutationTime(long elapsed_time) {
        mutation_time.push_back(elapsed_time);
    }

    void recordEvaluationTime(long elapsed_time) {
        evaluation_time.push_back(elapsed_time);
    }

    void recordMergeTime(long elapsed_time) {
        merge_time.push_back(elapsed_time);
    }

    void recordNonSerialTime(long elapsed_time) {
        non_serial_time.push_back(elapsed_time);
    }
        
    void recordSerialTime(long elapsed_time) {
        serial_time.push_back(elapsed_time);
    }

    void recordTotalTime(long elapsed_time){
        total_time = elapsed_time;
    }

    void writeTimesToFile(const string& filename, int num_workers) {
        convertTimesTotalToPhase();
        recordStatsPhases();

        ofstream outfile(filename, ios::app);

        if (!outfile.is_open()) {
            cerr << "Error: Unable to open file " << filename << " for writing." << endl;
            return;
        }

        outfile << "----- Recording run times -----" << endl;
        outfile << "Number of workers: " << num_workers << endl;
        outfile << "Initialization time: " << initialization_time << endl;
        if (!parallel) {
            outfile << "Selection average time: " << phase_stats[0].first << " +- " << phase_stats[0].second << endl;
            outfile << "Crossover average time: " << phase_stats[1].first << " +- " << phase_stats[1].second << endl;
            outfile << "Mutation average time: " << phase_stats[2].first << " +- " << phase_stats[2].second << endl;
            outfile << "Evaluation average time: " << phase_stats[3].first << " +- " << phase_stats[3].second << endl;
            outfile << "Merge average time: " << phase_stats[4].first << " +- " << phase_stats[4].second << endl;
        } else {
            outfile << "Serial average time: " << phase_stats[0].first << " +- " << phase_stats[0].second << endl;
            outfile << "Non-serial average time: " << phase_stats[1].first << " +- " << phase_stats[1].second << endl;

            outfile << "Load balancing statistics: " << endl;
            outfile << "Min load (averaged over generations): " << phase_stats[2].first << " +- " << phase_stats[2].second << endl;
            outfile << "Max load (averaged over generations): " << phase_stats[3].first << " +- " << phase_stats[3].second << endl;
            outfile << "Mean load (averaged over generations): " << phase_stats[4].first << " +- " << phase_stats[4].second << endl;
            outfile << "Standard deviation of load (averaged over generations): " << phase_stats[5].first << " +- " << phase_stats[5].second << endl;

            outfile << "Parallelization statistics: " << endl;
            outfile << "Average load imbalance (max - mean): " << phase_stats[3].first - phase_stats[4].first << endl;
            outfile << "Total load imbalance (max - mean): " << accumulate(max_loads.begin(), max_loads.end(), 0L) - accumulate(mean_loads.begin(), mean_loads.end(), 0L) << endl;
            outfile << "Total serial time (including initialization): " << accumulate(serial_time.begin(), serial_time.end(), 0L) + initialization_time << endl;
            outfile << "Total non-serial time: " << accumulate(non_serial_time.begin(), non_serial_time.end(), 0L) << endl;
        }       

        outfile << "\nTotal time: " << total_time << '\n' << endl;

        cout << "Time statistics of the run have been written to file " << filename << " successfully." << endl;

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
    int num_generations;

    long initialization_time;
    long total_time;

    // Sequential timings
    vector<long> selection_time;
    vector<long> crossover_time;
    vector<long> mutation_time;
    vector<long> evaluation_time;
    vector<long> merge_time;

    // Parallel timings
    vector<long> serial_time;
    vector<long> non_serial_time;

    vector<pair<double, double>> phase_stats;

    vector<long> min_loads;
    vector<long> max_loads;
    vector<long> mean_loads;
    vector<long> std_loads;


    pair<double, double> calculatePhaseStats(const vector<long>& time_vector) {
        
        long sum = accumulate(time_vector.begin(), time_vector.end(), 0L);

        double mean = sum / time_vector.size();

        long accum = 0L;
        for (const long& d : time_vector) {
            accum += (d - mean) * (d - mean);
        }
        double stdev = sqrt(accum / (time_vector.size() - 1));

        pair<double, double> stats = make_pair(mean, stdev);
        return stats;
    }

    void recordStatsPhases() {
        if (!parallel){
            phase_stats.push_back(calculatePhaseStats(selection_time));
            phase_stats.push_back(calculatePhaseStats(crossover_time));
            phase_stats.push_back(calculatePhaseStats(mutation_time));
            phase_stats.push_back(calculatePhaseStats(evaluation_time));
            phase_stats.push_back(calculatePhaseStats(merge_time));
        }
        else {
            phase_stats.push_back(calculatePhaseStats(serial_time));
            phase_stats.push_back(calculatePhaseStats(non_serial_time));
            phase_stats.push_back(calculatePhaseStats(min_loads));
            phase_stats.push_back(calculatePhaseStats(max_loads));
            phase_stats.push_back(calculatePhaseStats(mean_loads));
            phase_stats.push_back(calculatePhaseStats(std_loads));
        }
    }

    void convertTimesTotalToPhase(){

        for(int i = 0; i < num_generations; i++){
            if (!parallel){
                merge_time[i] = merge_time[i] - evaluation_time[i];
                evaluation_time[i] = evaluation_time[i] - mutation_time[i];
                mutation_time[i] = mutation_time[i] - crossover_time[i];
                crossover_time[i] = crossover_time[i] - selection_time[i];
            }
            else {
                serial_time[i] = serial_time[i] - non_serial_time[i];
            }
        }
    }
};


#endif
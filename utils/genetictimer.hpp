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
    
    GeneticTimer(bool parallel) : parallel(parallel) {}

    void start_total() {
        start_total_time = chrono::steady_clock::now();
    }

    void start() {
        start_time = chrono::steady_clock::now();
    }

    void stop() {
        end_time = chrono::steady_clock::now();
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
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        initialization_time = elapsed_time;
    }
    void recordSelectionTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        selection_time.push_back(elapsed_time);
    }

    void recordCrossoverTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        crossover_time.push_back(elapsed_time);
    }

    void recordMutationTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        mutation_time.push_back(elapsed_time);
    }

    void recordEvaluationTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        evaluation_time.push_back(elapsed_time);
    }

    void recordMergeTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        merge_time.push_back(elapsed_time);
    }

    void recordOffspringParTime() {
        stop();
        auto elapsed_time = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        offspringpar_time.push_back(elapsed_time);
    }

    void recordTotalTime(){
        stop();
        total_time = chrono::duration_cast<chrono::microseconds>(end_time - start_total_time).count();
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
            outfile << "Selection average time: " << phase_stats[0].first << "+- " << phase_stats[0].second << endl;
            outfile << "Crossover average time: " << phase_stats[1].first << "+- " << phase_stats[1].second << endl;
            outfile << "Mutation average time: " << phase_stats[2].first << "+- " << phase_stats[2].second << endl;
            outfile << "Evaluation average time: " << phase_stats[3].first << "+- " << phase_stats[3].second << endl;
            outfile << "Merge average time: " << phase_stats[4].first << "+- " << phase_stats[4].second << endl;
        } else {
            outfile << "Worker average time: " << phase_stats[0].first << endl;
            outfile << "Merge average time: " << phase_stats[1].first << endl;
            outfile << "Load balancing:" << endl;
            outfile << "Min load time among workers (average across generations): " << phase_stats[2].first << "+- " << phase_stats[2].second << endl;
            outfile << "Max load time among workers (average across generations): " << phase_stats[3].first << "+- " << phase_stats[3].second << endl;
            outfile << "Mean of load times among workers (average across generations): " << phase_stats[4].first << "+- " << phase_stats[4].second << endl;
            outfile << "Std of load times among workers (average across generations): " << phase_stats[5].first << "+- " << phase_stats[5].second << endl;
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

    long initialization_time;
    long total_time;

    vector<long> selection_time;
    vector<long> crossover_time;
    vector<long> mutation_time;
    vector<long> evaluation_time;
    vector<long> merge_time;
    vector<long> offspringpar_time;

    vector<pair<double, double>> phase_stats;

    vector<long> min_loads;
    vector<long> max_loads;
    vector<long> mean_loads;
    vector<long> std_loads;

    chrono::steady_clock::time_point start_total_time;
    chrono::steady_clock::time_point start_time;
    chrono::steady_clock::time_point end_time;

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
            phase_stats.push_back(calculatePhaseStats(offspringpar_time));
            phase_stats.push_back(calculatePhaseStats(merge_time));
            phase_stats.push_back(calculatePhaseStats(min_loads));
            phase_stats.push_back(calculatePhaseStats(max_loads));
            phase_stats.push_back(calculatePhaseStats(mean_loads));
            phase_stats.push_back(calculatePhaseStats(std_loads));
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
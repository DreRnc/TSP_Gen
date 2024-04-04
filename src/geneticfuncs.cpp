#include <iostream>
#include <algorithm>
#include "geneticfuncs.hpp"
#include "utilfuncs.hpp"

using namespace std;

vector<Individual> initialize_population(const int population_size, const int chr_length, mt19937& gen) {
    
    vector<Individual> population(population_size);
    
    for (int i = 0; i < population_size; ++i) {
        Individual& individual = population[i];
        individual.chr = generate_shuffled_sequence(chr_length, gen);
    }
    
    return population;
}

double population_total_fitness(const vector<Individual>& population){
    return accumulate(population.begin(), population.end(), 0.0, [](double total, const Individual& individual) {
                               return total + individual.fitness;
                           });
}

void evaluate_population(vector<Individual>& population, const Matrix& distance_matrix) {
    for (Individual& individual : population) {
        double score = 0.0;
        const vector<int>& ids = individual.chr;

        for (size_t i = 1; i < ids.size(); ++i) {
            score += distance_matrix[ids[i - 1]][ids[i]];
        }

        score += distance_matrix[ids.back()][ids.front()];

        individual.score = score;
        individual.fitness = 1.0 / individual.score;
    }
}

vector<Individual> select_parents(const vector<Individual>& population, const int num_parents, mt19937& gen) {
    vector<Individual> parents;

    double total_fitness = population_total_fitness(population);
    uniform_real_distribution<double> dist(0.0, total_fitness);

    while (parents.size() < num_parents) {
        double random_fitness = dist(gen);
        double accumulated_fitness = 0.0;
        for (const Individual& individual : population) {
            accumulated_fitness += 1.0 / individual.score;
            if (accumulated_fitness >= random_fitness) {
                parents.push_back(individual);
                break;
            }
        }
    }

    return parents;
}

vector<Individual> crossover_population(const vector<Individual>& population, mt19937& gen){
    vector<Individual> children;
    for(int i = 1; i < population.size(); i+=2){
        auto [child1, child2] = crossover(population[i-1], population[i], gen);
        children.push_back(child1);
        children.push_back(child2);
    }
    // If the population is odd, cross the last one (which would be left out, with the first)
    // Discard one of the two so that the offspring size is always 100 
    // This last choice is mainly done to simply compare correctly the merge times
    if (population.size() % 2){
        auto [child1, _] = crossover(population[population.size()-1], population[0], gen);
        children.push_back(child1);
    }
    return children; 
}

pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, mt19937& gen) {
    int size = parent1.chr.size();

    uniform_int_distribution<int> dist(0, size - 1);
    int cutpoint1 = dist(gen);
    int cutpoint2 = dist(gen);

    Individual child1, child2;
    child1 = _cross(parent1, parent2, cutpoint1, cutpoint2, size);
    child2 = _cross(parent2, parent1, cutpoint1, cutpoint2, size);

    pair<Individual, Individual> children = make_pair(child1, child2);

    return children;
}

Individual _cross(const Individual& parent1, const Individual& parent2, const int cutpoint1, const int cutpoint2, const int size){
    Individual child;

    for (int i = 0; i < size; ++i) {
        // Crossing the middle:     -1 -1 -1 || 4 5 6 || -1 -1 -1
        if (cutpoint1 < cutpoint2 && i > cutpoint1 && i < cutpoint2) {
            child.chr.push_back(parent1.chr[i]);
        }
        // Crossing the ends:       1 2 3 || -1 -1 -1 || 7 8 9 
        else if (cutpoint1 > cutpoint2 && !(i < cutpoint1 && i > cutpoint2)) {
            child.chr.push_back(parent1.chr[i]);
        }
        else {
            child.chr.push_back(-1);
        }
    }

    for (int i = 0; i < size; ++i) {
        if (find(child.chr.begin(), child.chr.end(), parent2.chr[i]) == child.chr.end()) {
            for (int j = 0; j < size; ++j) {
                if (child.chr[j] == -1) {
                    child.chr[j] = parent2.chr[i];
                    break;
                }
            }
        }
    }
    return child;
}

void mutate(vector<Individual>& population, mt19937& gen) {
    uniform_int_distribution<int> dist(0, population[0].chr.size() - 1);

    for (auto& individual : population) {
        int pos1 = dist(gen);
        int pos2 = dist(gen);

        swap(individual.chr[pos1], individual.chr[pos2]);
    }
}

void merge(vector<Individual>& population, vector<Individual>& offspring, mt19937& gen) {
    int population_size = population.size();
    
    vector<Individual> merged_population;
    merged_population.reserve(population_size + offspring.size());

    population.insert(population.end(), offspring.begin(), offspring.end());

    sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });

    for (int i = 0; i < population_size; ++i) {
        merged_population.push_back(population[i]);
    }

    population = merged_population;
}
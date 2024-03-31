#include <iostream>
#include "geneticfuncs.hpp"
#include "utilfuncs.hpp"

using namespace std;

vector<Individual> initialize_population(int population_size, const int chr_length, mt19937& gen) {
    
    vector<Individual> population(population_size);
    
    for (int i = 0; i < population_size; ++i) {
        Individual& individual = population[i];
        individual.chr = generate_shuffled_sequence(chr_length, gen);
    }
    
    return population;
}

double population_total_fitness(const vector<Individual>& population){
    double total_fitness = 0.0;
    for (const Individual& individual : population) {
        total_fitness += 1.0 / individual.score;
    }
    return total_fitness;
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

vector<Individual> select_parents(const vector<Individual>& population, int num_parents, mt19937& gen) {
    vector<Individual> parents;

    double total_fitness = population_total_fitness(population);
    uniform_real_distribution<double> dist(0.0, total_fitness);

    // Select parents using roulette wheel selection
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

Individual _cross(const Individual& parent1, const Individual& parent2, int cutpoint1, int cutpoint2, int size){
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

    population.insert(population.end(), offspring.begin(), offspring.end());

    sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
        return a.fitness > b.fitness;
    });

    vector<Individual> merged_population;

    for (int i = 0; i < population_size; ++i) {
        merged_population.push_back(population[i]);
    }

    population = merged_population;
}


/*
vector<int> pmx(const vector<int>& parent1, const vector<int>& parent2, int cutPoint1, int cutPoint2) {

    if (parent1.size() != parent2.size() || cutPoint1 < 0 || cutPoint2 < 0 ||
        cutPoint1 >= parent1.size() || cutPoint2 >= parent1.size()) {
        throw invalid_argument("Invalid input parameters.");
    }

    vector<int> child(parent1.size(), -1);

    for (int i = cutPoint1; i <= cutPoint2; ++i) {
        child[i] = parent1[i];
    }

    unordered_map<int, int> mapping;
    for (int i = cutPoint1; i <= cutPoint2; ++i) {
        mapping[parent1[i]] = parent2[i];
    }

    for (int i = 0; i < parent1.size(); ++i) {
        if (i < cutPoint1 || i > cutPoint2) {
            int current = parent2[i];
            while (mapping.find(current) != mapping.end()) {
                current = mapping[current];
            }
            child[i] = current;
        }
    }
    return child;
}

// Function to perform PMX crossover and produce two children
pair<vector<int>, vector<int>> pmxCrossover(const vector<int>& parent1, const vector<int>& parent2) {
    int size = parent1.size();

    // Randomly select cut points
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, size - 1);
    int cutPoint1 = dist(gen);
    int cutPoint2 = dist(gen);

    // Ensure cutPoint2 > cutPoint1
    if (cutPoint1 > cutPoint2) {
        swap(cutPoint1, cutPoint2);
    }

    // Perform PMX crossover to produce two children
    vector<int> child1 = pmx(parent1, parent2, cutPoint1, cutPoint2);
    vector<int> child2 = pmx(parent2, parent1, cutPoint1, cutPoint2);

    return make_pair(child1, child2);
}
*/
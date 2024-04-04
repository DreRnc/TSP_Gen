#ifndef GENETIC_FUNCS_HPP
#define GENETIC_FUNCS_HPP

#include <vector>
#include <random>
#include <utility>

using namespace std;

struct Individual{
    vector<int> chr;
    double score;
    double fitness;
};

using Matrix = vector<vector<double>>;

vector<Individual> initialize_population(const int population_size, const int route_length, mt19937& gen);
void evaluate_population(vector<Individual>& population, const Matrix& distance_matrix);
vector<Individual> select_parents(const vector<Individual>& population, const int num_parents, mt19937& gen);
Individual _cross(const Individual& parent1, const Individual& parent2, const int cutpoint1, const int cutpoint2, const int size);
pair<Individual, Individual> crossover(const Individual& parent1, const Individual& parent2, mt19937& gen);
vector<Individual> crossover_population(const vector<Individual>& population, mt19937& gen);
void mutate(vector<Individual>& population, mt19937& gen);
void merge(vector<Individual>& population, const vector<Individual>& offspring);

#endif

#ifndef DISTANCE_FUNCS_HPP
#define DISTANCE_FUNCS_HPP

#include <vector>

using namespace std;
using Matrix = vector<vector<double>>;

struct City {
    int id;
    double x;
    double y;
};
double compute_distance(City c1, City c2);

vector<City> generate_city_vector(string data_path);

Matrix generate_distance_matrix(vector<City> cities);

#endif
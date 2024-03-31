#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include "distancefuncs.hpp"

double compute_distance(City c1, City c2){
    double distance = sqrt(pow(c1.x - c2.x, 2) + pow(c1.y - c2.y, 2));
    return distance;
}

vector<City> generate_city_vector(string data_path) {

    ifstream file(data_path);
    if (!file.is_open()) {
        cerr << "Error opening file." << endl;
    }
    

    vector<City> cities;
    City city;

    string line;
    while (getline(file, line)) {

        if (!line.empty() && isdigit(line[0])) {
            istringstream iss(line);
            if (!(iss >> city.id >> city.x >> city.y)) {
                cerr << "Error reading city data." << endl;
                continue;
            }
            cities.push_back(city);
        }
    }

    file.close();

    return cities;
}

Matrix generate_distance_matrix(vector<City> cities){
    
    Matrix distance_matrix(cities.size(), vector<double>(cities.size(), 0.0));

    for (size_t i = 0; i < cities.size(); ++i) {
        for (size_t j = i + 1; j < cities.size(); ++j) {
            double distance = compute_distance(cities[i], cities[j]);
            distance_matrix[i][j] = distance;
            distance_matrix[j][i] = distance;
        }
    }

    return distance_matrix;
}
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using FloatMatrix = vector<vector<float>>;

struct City {
    int id;
    float x;
    float y;
};

float compute_distance(City c1, City c2){
    float distance = sqrt()
}
vector<City> generate_city_vector(string data_path) {
    ifstream file(data_path);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    vector<City> cities;
    City city;


    while (file >> city.id >> city.x >> city.y) {
        cities.push_back(city);
    }

    file.close();

    return cities;
}



FloatMatrix generate_distance_matrix(string data_path){
	vector<City> cities = generate_city_vector(data_path);

}
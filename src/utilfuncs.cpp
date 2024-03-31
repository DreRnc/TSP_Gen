#include <vector>
#include <random>
#include <iostream>
#include <algorithm>

using namespace std;

vector<int> generate_shuffled_sequence(const int n, mt19937& gen) {

    vector<int> sequence(n);
    for (int i = 0; i < n; ++i) {
        sequence[i] = i;
    }
    cout <<'1'<<endl;

    shuffle(sequence.begin(), sequence.end(), gen);
    /*for (int i = n - 1; i > 0; --i) {
        uniform_int_distribution<int> dist(0, i);
        int j = dist(gen);
        swap(sequence[i], sequence[j]);
    }
    */
    cout <<'2'<<endl;
    return sequence;
}
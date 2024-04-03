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
    // Fisher Yates alg
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1); 
        swap(sequence[i], sequence[j]); 
    } 
    return sequence;
}

tuple<long, long, long, long> vec_stats(const vector<long>& v) {
    long sum = accumulate(v.begin(), v.end(), 0L);

    long mean = sum / v.size();

    long minimum = *min_element(v.begin(), v.end());
    long maximum = *max_element(v.begin(), v.end());

    long accum = 0L;
    for (const long& d : v) {
        accum += (d - mean) * (d - mean);
    }
    long double stdev = sqrt(accum / (v.size() - 1));

    return make_tuple(minimum, maximum, mean, static_cast<long>(stdev));
}
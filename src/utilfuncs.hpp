#ifndef UTIL_FUNCS_HPP
#define UTIL_FUNCS_HPP

#include <vector>

vector<int> generate_shuffled_sequence(const int n, mt19937& gen);
tuple<long, long, long, long> vec_stats(const vector<long>& v);

#endif
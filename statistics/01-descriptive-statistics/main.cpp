#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
struct Summary { double mean, median, population_variance; };
Summary summarize(std::vector<double> v) {
    if (v.empty()) throw std::invalid_argument("empty data");
    for (double x : v) if (!std::isfinite(x)) throw std::invalid_argument("non-finite value");
    std::sort(v.begin(), v.end());
    const auto n = v.size();
    double mean = std::accumulate(v.begin(), v.end(), 0.0) / n;
    double median = n % 2 ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
    double squares = 0;
    for (double x : v) squares += (x - mean) * (x - mean);
    return {mean, median, squares / n};
}
#ifndef STAT_TEST
int main() {
    auto s = summarize({2, 4, 4, 6, 9});
    std::cout << "mean=" << s.mean << " median=" << s.median
              << " population_variance=" << s.population_variance << '\n';
}
#endif

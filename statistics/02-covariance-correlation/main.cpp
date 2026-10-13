#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

double mean(const std::vector<double>& x) {
    if (x.empty()) throw std::invalid_argument("empty sample");
    double sum = 0;
    for (double v : x) sum += v;
    return sum / x.size();
}

double sample_covariance(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2)
        throw std::invalid_argument("samples must have equal size >= 2");
    const double mx = mean(x), my = mean(y);
    double total = 0;
    for (std::size_t i = 0; i < x.size(); ++i)
        total += (x[i] - mx) * (y[i] - my);
    return total / (x.size() - 1);
}

double pearson_correlation(const std::vector<double>& x, const std::vector<double>& y) {
    const double covariance = sample_covariance(x, y);
    const double variance_x = sample_covariance(x, x);
    const double variance_y = sample_covariance(y, y);
    if (variance_x == 0 || variance_y == 0)
        throw std::invalid_argument("correlation undefined for constant sample");
    return covariance / std::sqrt(variance_x * variance_y);
}

#ifndef CORRELATION_TEST
int main() {
    const std::vector<double> x{1, 2, 3, 4};
    const std::vector<double> y{2, 4, 6, 8};
    std::cout << std::fixed << std::setprecision(3)
              << "sample covariance=" << sample_covariance(x, y) << '\n'
              << "Pearson r=" << pearson_correlation(x, y) << '\n';
}
#endif

#define CORRELATION_TEST
#include "main.cpp"
#include <cassert>
int main() {
    const std::vector<double> x{1, 2, 3};
    const std::vector<double> y{2, 4, 6};
    const std::vector<double> z{6, 4, 2};
    assert(std::abs(sample_covariance(x, y) - 2.0) < 1e-12);
    assert(std::abs(pearson_correlation(x, y) - 1.0) < 1e-12);
    assert(std::abs(pearson_correlation(x, z) + 1.0) < 1e-12);
    bool rejected = false;
    try { pearson_correlation(x, {1, 1, 1}); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

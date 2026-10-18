#include "bootstrap_mean.hpp"
#include <iomanip>
#include <iostream>
#include <vector>
int main() {
    const std::vector<double> measurements{8, 10, 12, 14, 16};
    const auto result = statistics::bootstrap_mean_interval(measurements, 10000, 0.95, 12345);
    std::cout << std::fixed << std::setprecision(6)
              << "Sample mean: " << result.sample_mean << '\n'
              << "Bootstrap SE: " << result.bootstrap_standard_error << '\n'
              << "Percentile 95% CI: [" << result.lower << ", " << result.upper << "]\n"
              << "Resamples: " << result.resamples << '\n';
}

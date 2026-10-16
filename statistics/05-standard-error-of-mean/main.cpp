#include "standard_error.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    const std::vector<double> measurements{8.0, 10.0, 12.0, 14.0, 16.0};
    const auto result = statistics::summarize_mean(measurements);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Observations: " << result.sample_size << '\n';
    std::cout << "Sample mean: " << result.mean << '\n';
    std::cout << "Sample standard deviation: "
              << result.sample_standard_deviation << '\n';
    std::cout << "Standard error of the mean: "
              << result.standard_error << '\n';
}

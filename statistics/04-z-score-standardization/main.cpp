#include "z_scores.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    const std::vector<double> observations{10.0, 12.0, 14.0, 16.0, 18.0};
    const auto summary = statistics::describe_sample(observations);
    const auto scores = statistics::z_scores(observations);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Sample mean: " << summary.mean << '\n';
    std::cout << "Sample standard deviation: " << summary.sample_standard_deviation << '\n';
    for (std::size_t index = 0; index < observations.size(); ++index) {
        std::cout << "Value " << observations[index] << " -> z-score "
                  << scores[index] << '\n';
    }
}

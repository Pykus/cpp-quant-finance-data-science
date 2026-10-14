#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

double portfolio_return(const std::vector<double>& weights,
                        const std::vector<double>& returns) {
    if (weights.empty() || weights.size() != returns.size())
        throw std::invalid_argument("weights and returns must have equal nonzero size");
    double sum_weights = 0.0;
    double result = 0.0;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        if (!std::isfinite(weights[i]) || !std::isfinite(returns[i]) || weights[i] < 0)
            throw std::invalid_argument("invalid weight or return");
        sum_weights += weights[i];
        result += weights[i] * returns[i];
    }
    if (std::abs(sum_weights - 1.0) > 1e-9)
        throw std::invalid_argument("weights must sum to 1");
    return result;
}

#ifndef PORTFOLIO_TEST
int main() {
    std::cout << std::fixed << std::setprecision(2)
              << "Portfolio return: "
              << portfolio_return({0.60, 0.40}, {0.10, -0.05}) * 100
              << "%\n";
}
#endif

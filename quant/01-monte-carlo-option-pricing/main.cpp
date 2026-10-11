#include "pricing.hpp"
#include <iomanip>
#include <iostream>
int main() {
    const quant::Option option{100.0, 100.0, 0.05, 0.2, 1.0};
    const auto estimate = quant::monte_carlo_call(option, 100000, 42);
    std::cout << std::fixed << std::setprecision(6)
              << "black_scholes=" << quant::black_scholes_call(option) << '\n'
              << "monte_carlo=" << estimate.price << '\n'
              << "standard_error=" << estimate.standard_error << '\n'
              << "ci95_lower=" << estimate.price - 1.96 * estimate.standard_error << '\n'
              << "ci95_upper=" << estimate.price + 1.96 * estimate.standard_error << '\n'
              << "paths=" << estimate.paths << '\n';
}

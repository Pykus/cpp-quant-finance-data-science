#include "pricing.hpp"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

int main() {
    using namespace quant::antithetic;
    const EuropeanCall option{100, 100, 0.05, 0.20, 1.0};
    constexpr std::size_t pairs = 100'000;
    constexpr std::uint64_t seed = 2026;
    const auto plain = estimate(option, pairs, seed, Method::independent);
    const auto anti = estimate(option, pairs, seed, Method::antithetic);
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Black-Scholes reference: " << black_scholes(option) << '\n';
    std::cout << "Independent MC: " << plain.price << " (SE " << plain.standard_error
              << ", CI " << plain.low95 << " .. " << plain.high95 << ")\n";
    std::cout << "Antithetic MC: " << anti.price << " (SE " << anti.standard_error
              << ", CI " << anti.low95 << " .. " << anti.high95 << ")\n";
    std::cout << "Independent observations: " << plain.observations << " vs "
              << anti.observations << "; payoff evaluations per method: "
              << anti.payoffs << '\n';
}

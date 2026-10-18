#include "risk.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <span>

int main() {
    // Ten synthetic one-day returns, each recorded as a decimal fraction.
    constexpr std::array returns{
        -0.10, -0.06, -0.04, -0.02, 0.00,
         0.01,  0.02,  0.03,  0.04, 0.05};

    const auto risk = quant::historical_tail_risk(
        std::span<const double>{returns.data(), returns.size()}, 0.80);
    const auto dollars = quant::scale_to_money(risk, 10'000.0);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Observations: " << risk.observations << '\n';
    std::cout << "Confidence: " << risk.confidence * 100.0 << "%\n";
    std::cout << "Historical VaR: " << risk.value_at_risk * 100.0 << "%\n";
    std::cout << "Historical expected shortfall: "
              << risk.expected_shortfall * 100.0 << "%\n";
    std::cout << "VaR at 10000 currency units: "
              << dollars.value_at_risk << '\n';
    std::cout << "ES at 10000 currency units: "
              << dollars.expected_shortfall << '\n';
}
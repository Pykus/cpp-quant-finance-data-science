#include "sharpe.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Five fictional months; all numbers are decimal returns, not percentages.
    const std::vector<double> asset_a{0.04, -0.02, 0.06, 0.00, 0.01};
    const std::vector<double> asset_b{0.00, 0.00, 0.00, 0.00, 0.01};
    const double weight_a = 0.50;
    const double monthly_risk_free = 0.002;

    const auto portfolio =
        lesson06::two_asset_returns(asset_a, asset_b, weight_a);
    const auto result = lesson06::analyze_sharpe(portfolio, monthly_risk_free);

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Observations: " << result.observations << '\n';
    std::cout << "Mean monthly return: " << result.mean_return * 100 << "%\n";
    std::cout << "Sample monthly standard deviation: "
              << result.sample_standard_deviation * 100 << "%\n";
    std::cout << "Monthly risk-free rate: " << result.risk_free_return * 100 << "%\n";
    std::cout << "Mean monthly excess return: "
              << result.mean_excess_return * 100 << "%\n";
    std::cout << "Sample monthly Sharpe ratio: " << result.sharpe_ratio << '\n';
}

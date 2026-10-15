#include "risk.hpp"

#include <iomanip>
#include <iostream>

int main() {
    // Three hypothetical monthly returns, aligned by month.
    const std::vector<double> asset_a{0.10, -0.02, 0.06};
    const std::vector<double> asset_b{0.00, 0.04, 0.02};
    const RiskSummary risk = analyze_portfolio(asset_a, asset_b, 0.60);

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Mean portfolio return: " << 100.0 * risk.mean_portfolio << "%\n";
    std::cout << "Sample portfolio standard deviation: "
              << 100.0 * risk.standard_deviation << "%\n";
    std::cout << std::setprecision(6)
              << "Sample covariance (A,B): " << risk.covariance << "\n";
}

#include "drawdown.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Entirely fictional monthly returns, written as decimal fractions.
    const std::vector<double> monthly_returns{0.20, -0.25, 0.50, -0.20, 0.10};
    const auto report = quant::analyze_drawdowns(monthly_returns, 100.0);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Period | Return (%) | Wealth | Peak | Drawdown (%)\n";
    for (const auto& row : report.path) {
        std::cout << row.period << " | " << row.period_return * 100.0
                  << " | " << row.wealth << " | " << row.running_peak
                  << " | " << row.drawdown * 100.0 << '\n';
    }
    std::cout << "Final wealth: " << report.path.back().wealth << '\n';
    std::cout << "Cumulative return: " << report.cumulative_return * 100.0 << "%\n";
    std::cout << "Maximum drawdown: " << report.maximum_drawdown * 100.0 << "%\n";
    std::cout << "Worst decline: period " << report.peak_period
              << " to period " << report.trough_period << '\n';
}

#ifndef QUANT_07_DRAWDOWN_HPP
#define QUANT_07_DRAWDOWN_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace quant {

// A return of 0.20 means +20%; -0.25 means -25%.
struct PeriodResult {
    std::size_t period;
    double period_return;
    double wealth;
    double running_peak;
    double drawdown;
};

struct DrawdownSummary {
    std::vector<PeriodResult> path; // Includes period zero (before any return).
    double cumulative_return;
    double maximum_drawdown;
    std::size_t peak_period;   // Peak preceding the worst drawdown.
    std::size_t trough_period; // Lowest point of that drawdown.
};

// Reinvest the entire portfolio after each equally spaced observation.
// This is an educational calculation, not a forecast or a trading strategy.
[[nodiscard]] inline DrawdownSummary analyze_drawdowns(
    const std::vector<double>& returns,
    double initial_wealth = 100.0) {
    if (!std::isfinite(initial_wealth) || initial_wealth <= 0.0) {
        throw std::invalid_argument("initial wealth must be finite and positive");
    }
    if (returns.empty()) {
        throw std::invalid_argument("provide at least one periodic return");
    }

    DrawdownSummary result{{}, 0.0, 0.0, 0, 0};
    result.path.reserve(returns.size() + 1);
    result.path.push_back({0, 0.0, initial_wealth, initial_wealth, 0.0});

    double wealth = initial_wealth;
    double running_peak = initial_wealth;
    std::size_t current_peak_period = 0;

    for (std::size_t i = 0; i < returns.size(); ++i) {
        const double period_return = returns[i];
        if (!std::isfinite(period_return) || period_return < -1.0) {
            throw std::invalid_argument("each return must be finite and at least -1");
        }

        wealth *= 1.0 + period_return;
        if (!std::isfinite(wealth)) {
            throw std::overflow_error("wealth is not representable as double");
        }
        if (wealth > running_peak) {
            running_peak = wealth;
            current_peak_period = i + 1;
        }

        const double drawdown = std::clamp(
            (running_peak - wealth) / running_peak, 0.0, 1.0);
        result.path.push_back({i + 1, period_return, wealth, running_peak, drawdown});
        if (drawdown > result.maximum_drawdown) {
            result.maximum_drawdown = drawdown;
            result.peak_period = current_peak_period;
            result.trough_period = i + 1;
        }
    }

    result.cumulative_return = wealth / initial_wealth - 1.0;
    if (!std::isfinite(result.cumulative_return)) {
        throw std::overflow_error("cumulative return is not representable as double");
    }
    return result;
}

} // namespace quant

#endif // QUANT_07_DRAWDOWN_HPP

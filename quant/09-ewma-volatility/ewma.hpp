#pragma once

#include <cmath>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

namespace quant {

// Risk estimated at the END of day t, using returns observed through day t.
// It is a forecast for the NEXT day, not an explanation of today's realised loss.
struct RiskSnapshot {
    std::size_t day;
    double variance;
    double daily_volatility;
    double annualized_volatility;
    double next_day_var95;
};

inline constexpr double normal_95_quantile = 1.6448536269514722;

// Decimal returns: +1% is 0.01. Initial variance = first observed return squared.
// No estimation is possible without a return; reject invalid or overflowing inputs.
[[nodiscard]] inline std::vector<RiskSnapshot> ewma_risk(
    std::span<const double> daily_returns,
    double decay,
    double portfolio_value,
    double trading_days_per_year = 252.0) {

    if (daily_returns.empty()) {
        throw std::invalid_argument("At least one daily return is required");
    }
    if (!std::isfinite(decay) || decay < 0.0 || decay >= 1.0) {
        throw std::invalid_argument("Decay must be finite and in [0, 1)");
    }
    if (!std::isfinite(portfolio_value) || portfolio_value < 0.0) {
        throw std::invalid_argument("Portfolio value must be finite and nonnegative");
    }
    if (!std::isfinite(trading_days_per_year) || trading_days_per_year <= 0.0) {
        throw std::invalid_argument("Trading days must be finite and positive");
    }

    std::vector<RiskSnapshot> result;
    result.reserve(daily_returns.size());
    double variance = 0.0;

    for (std::size_t day = 0; day < daily_returns.size(); ++day) {
        const double observation = daily_returns[day];
        if (!std::isfinite(observation)) {
            throw std::invalid_argument("Returns must be finite");
        }
        const double squared = observation * observation;
        if (!std::isfinite(squared)) {
            throw std::overflow_error("A squared return overflowed");
        }
        variance = day == 0 ? squared
                            : decay * variance + (1.0 - decay) * squared;
        if (!std::isfinite(variance)) {
            throw std::overflow_error("EWMA variance overflowed");
        }
        const double daily = std::sqrt(variance);
        const double annualized = daily * std::sqrt(trading_days_per_year);
        const double next_day_var95 = normal_95_quantile * daily * portfolio_value;
        if (!std::isfinite(annualized) || !std::isfinite(next_day_var95)) {
            throw std::overflow_error("The risk estimate overflowed");
        }
        result.push_back({day + 1, variance, daily, annualized, next_day_var95});
    }
    return result;
}

} // namespace quant

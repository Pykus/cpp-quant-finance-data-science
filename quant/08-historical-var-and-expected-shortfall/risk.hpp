#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>

namespace quant {

struct TailRisk {
    double confidence;
    double value_at_risk;
    double expected_shortfall;
    std::size_t observations;
};

struct MoneyRisk {
    double value_at_risk;
    double expected_shortfall;
};

// All values are fractions: a return of -0.04 means a 4% loss.
// The empirical VaR quantile uses nearest rank: ceil(confidence * n).
// Expected shortfall averages the worst (1-confidence)*n observations,
// including a fractional weight at the tail boundary if necessary.
[[nodiscard]] inline TailRisk historical_tail_risk(
    std::span<const double> simple_returns, double confidence) {
    if (!(confidence > 0.0 && confidence < 1.0)) {
        throw std::invalid_argument("confidence must lie strictly between 0 and 1");
    }
    if (simple_returns.empty()) {
        throw std::invalid_argument("at least one return is required");
    }

    std::vector<double> losses;
    losses.reserve(simple_returns.size());
    for (double value : simple_returns) {
        if (!std::isfinite(value) || value < -1.0) {
            throw std::invalid_argument("returns must be finite and at least -1");
        }
        losses.push_back(-value);
    }

    std::sort(losses.begin(), losses.end());
    const double count = static_cast<double>(losses.size());
    const auto rank = std::min(
        losses.size(),
        static_cast<std::size_t>(std::ceil(confidence * count)));
    const double var = losses[std::max(std::size_t{1}, rank) - 1];

    const double tail_mass = (1.0 - confidence) * count;
    double remaining = tail_mass;
    double es = 0.0;
    for (auto it = losses.rbegin();
         it != losses.rend() && remaining > 0.0; ++it) {
        const double weight = std::min(1.0, remaining);
        es += (weight / tail_mass) * (*it);
        remaining -= weight;
    }
    if (!std::isfinite(es)) {
        throw std::overflow_error("expected shortfall is not finite");
    }
    return {confidence, var, es, losses.size()};
}

[[nodiscard]] inline MoneyRisk scale_to_money(
    const TailRisk& risk, double portfolio_value) {
    if (!std::isfinite(portfolio_value) || portfolio_value <= 0.0) {
        throw std::invalid_argument("portfolio value must be positive and finite");
    }
    const double var = risk.value_at_risk * portfolio_value;
    const double es = risk.expected_shortfall * portfolio_value;
    if (!std::isfinite(var) || !std::isfinite(es)) {
        throw std::overflow_error("scaled loss is not finite");
    }
    return {var, es};
}

}  // namespace quant
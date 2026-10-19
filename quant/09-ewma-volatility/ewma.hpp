#pragma once
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace quant {
class EwmaVolatility final {
public:
    explicit EwmaVolatility(double decay, double initial_variance)
        : decay_(validate_decay(decay)), variance_(validate_variance(initial_variance)) {}

    [[nodiscard]] double decay() const noexcept { return decay_; }
    [[nodiscard]] double forecast_variance() const noexcept { return variance_; }
    [[nodiscard]] double forecast_volatility() const noexcept { return std::sqrt(variance_); }
    [[nodiscard]] std::size_t observations() const noexcept { return observations_; }
    [[nodiscard]] double half_life_observations() const noexcept {
        return std::log(0.5) / std::log(decay_);
    }
    [[nodiscard]] double annualized_volatility(double periods) const {
        if (!std::isfinite(periods) || periods <= 0.0)
            throw std::invalid_argument("periods must be positive and finite");
        const double result = variance_ * periods;
        if (!std::isfinite(result))
            throw std::overflow_error("annualized variance overflow");
        return std::sqrt(result);
    }

    // Update the NEXT period's forecast only AFTER observing today's return.
    void observe(double daily_return) {
        if (!std::isfinite(daily_return))
            throw std::invalid_argument("return must be finite");
        const double squared = daily_return * daily_return;
        if (!std::isfinite(squared))
            throw std::overflow_error("squared return overflow");
        const double next = decay_ * variance_ + (1.0 - decay_) * squared;
        if (!std::isfinite(next))
            throw std::overflow_error("EWMA variance overflow");
        if (observations_ == std::numeric_limits<std::size_t>::max())
            throw std::overflow_error("observation count overflow");
        variance_ = next;
        ++observations_;
    }
private:
    [[nodiscard]] static double validate_decay(double value) {
        if (!std::isfinite(value) || value <= 0.0 || value >= 1.0)
            throw std::invalid_argument("decay must lie strictly between 0 and 1");
        return value;
    }
    [[nodiscard]] static double validate_variance(double value) {
        if (!std::isfinite(value) || value < 0.0)
            throw std::invalid_argument("seed variance must be finite and nonnegative");
        return value;
    }
    double decay_;
    double variance_;
    std::size_t observations_{0};
};

struct NormalTailRisk95 {
    double value_at_risk_fraction;
    double expected_shortfall_fraction;
    double value_at_risk_amount;
    double expected_shortfall_amount;
};

// One-period, zero-mean normal-return model; NOT a historical percentile.
[[nodiscard]] inline NormalTailRisk95 normal_tail_risk_95(double sigma, double value) {
    if (!std::isfinite(sigma) || sigma < 0.0)
        throw std::invalid_argument("sigma must be finite and nonnegative");
    if (!std::isfinite(value) || value <= 0.0)
        throw std::invalid_argument("portfolio value must be positive and finite");
    constexpr double z95 = 1.6448536269514722;
    constexpr double normal_density = 0.10313564037537128;
    const double var_fraction = z95 * sigma;
    const double es_fraction = (normal_density / 0.05) * sigma;
    const double var_amount = value * var_fraction;
    const double es_amount = value * es_fraction;
    if (!std::isfinite(var_fraction) || !std::isfinite(es_fraction)
        || !std::isfinite(var_amount) || !std::isfinite(es_amount))
        throw std::overflow_error("risk amount overflow");
    return {var_fraction, es_fraction, var_amount, es_amount};
}
} // namespace quant

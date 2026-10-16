#ifndef LESSON06_SHARPE_HPP
#define LESSON06_SHARPE_HPP

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace lesson06 {

// Returns are decimal fractions: 0.02 means +2% for one period.
// Constant weights assume rebalancing before every observation period.
[[nodiscard]] inline std::vector<double> two_asset_returns(
    const std::vector<double>& asset_a,
    const std::vector<double>& asset_b,
    double weight_a) {
    if (asset_a.size() < 2 || asset_a.size() != asset_b.size()) {
        throw std::invalid_argument("provide at least two paired observations");
    }
    if (!std::isfinite(weight_a) || weight_a < 0.0 || weight_a > 1.0) {
        throw std::invalid_argument("weight must be between zero and one");
    }

    const double weight_b = 1.0 - weight_a;
    std::vector<double> result;
    result.reserve(asset_a.size());
    for (std::size_t i = 0; i < asset_a.size(); ++i) {
        if (!std::isfinite(asset_a[i]) || !std::isfinite(asset_b[i])) {
            throw std::invalid_argument("asset returns must be finite");
        }
        const double monthly = weight_a * asset_a[i] + weight_b * asset_b[i];
        if (!std::isfinite(monthly)) {
            throw std::invalid_argument("portfolio return is outside the numeric range");
        }
        result.push_back(monthly);
    }
    return result;
}

struct SharpeSummary {
    std::size_t observations;
    double mean_return;
    double risk_free_return;
    double mean_excess_return;
    double sample_variance;
    double sample_standard_deviation;
    double sharpe_ratio;
};

// All observations and the risk-free return must refer to the SAME period.
// The standard deviation is estimated with n-1 (the sample convention).
[[nodiscard]] inline SharpeSummary analyze_sharpe(
    const std::vector<double>& period_returns,
    double risk_free_per_period) {
    if (period_returns.size() < 2) {
        throw std::invalid_argument("at least two periods are required");
    }
    if (!std::isfinite(risk_free_per_period)) {
        throw std::invalid_argument("risk-free return must be finite");
    }

    long double sum = 0.0L;
    const double first = period_returns.front();
    bool varies = false;
    for (double value : period_returns) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("returns must be finite");
        }
        sum += static_cast<long double>(value);
        if (value != first) {
            varies = true;
        }
    }
    if (!varies) {
        throw std::domain_error("zero standard deviation: Sharpe ratio is undefined");
    }

    const auto n = period_returns.size();
    const long double mean = sum / static_cast<long double>(n);
    long double squared_deviations = 0.0L;
    for (double value : period_returns) {
        const long double difference = static_cast<long double>(value) - mean;
        squared_deviations += difference * difference;
    }
    const long double variance =
        squared_deviations / static_cast<long double>(n - 1);
    const long double standard_deviation = std::sqrt(variance);
    const long double excess = mean - static_cast<long double>(risk_free_per_period);
    const long double ratio = excess / standard_deviation;

    const SharpeSummary result{
        n,
        static_cast<double>(mean),
        risk_free_per_period,
        static_cast<double>(excess),
        static_cast<double>(variance),
        static_cast<double>(standard_deviation),
        static_cast<double>(ratio)
    };
    if (!std::isfinite(result.mean_return) ||
        !std::isfinite(result.mean_excess_return) ||
        !std::isfinite(result.sample_variance) ||
        !std::isfinite(result.sample_standard_deviation) ||
        !std::isfinite(result.sharpe_ratio)) {
        throw std::invalid_argument("values exceed the supported numeric range");
    }
    if (result.sample_standard_deviation == 0.0) {
        throw std::domain_error("standard deviation is too small to represent");
    }
    return result;
}

} // namespace lesson06

#endif // LESSON06_SHARPE_HPP

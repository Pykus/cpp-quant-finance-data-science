#pragma once

#include <cmath>
#include <cstddef>
#include <span>
#include <stdexcept>

namespace statistics11 {

struct Sample {
    std::size_t count;
    double mean;
    double sample_variance;
};

[[nodiscard]] inline Sample summarize(std::span<const double> values) {
    if (values.size() < 2) throw std::invalid_argument("At least two observations required");
    double mean = 0.0;
    double m2 = 0.0;
    std::size_t n = 0;
    for (double x : values) {
        if (!std::isfinite(x)) throw std::invalid_argument("Nonfinite observation");
        ++n;
        const double delta = x - mean;
        mean += delta / static_cast<double>(n);
        m2 += delta * (x - mean);
        if (!std::isfinite(mean) || !std::isfinite(m2)) {
            throw std::overflow_error("Numeric overflow");
        }
    }
    return {n, mean, m2 / static_cast<double>(n - 1)};
}

struct WelchResult {
    double mean_difference;
    double standard_error;
    double t_statistic;
    double approximate_degrees_of_freedom;
};

// Reports the statistic only, not a p-value or a causal conclusion.
[[nodiscard]] inline WelchResult welch(
    std::span<const double> group_a, std::span<const double> group_b) {
    const auto a = summarize(group_a);
    const auto b = summarize(group_b);
    const double aa = a.sample_variance / static_cast<double>(a.count);
    const double bb = b.sample_variance / static_cast<double>(b.count);
    const double error_sq = aa + bb;
    if (!std::isfinite(error_sq) || error_sq <= 0.0) {
        throw std::domain_error("Welch statistic requires positive finite standard error");
    }
    const double se = std::sqrt(error_sq);
    const double degrees_denom =
        aa * aa / static_cast<double>(a.count - 1) +
        bb * bb / static_cast<double>(b.count - 1);
    if (!(degrees_denom > 0) || !std::isfinite(degrees_denom)) {
        throw std::domain_error("Degrees-of-freedom calculation failed");
    }
    const double difference = a.mean - b.mean;
    const double t = difference / se;
    const double df = error_sq * error_sq / degrees_denom;
    if (!std::isfinite(t) || !std::isfinite(df)) {
        throw std::overflow_error("Welch calculation overflow");
    }
    return {difference, se, t, df};
}

}  // namespace statistics11

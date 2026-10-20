#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace statistics {
inline constexpr double z_95 = 1.959963984540054;
struct Interval {
    double rate, lower, upper;
    std::uint64_t successes, trials;
    [[nodiscard]] double width() const noexcept { return upper - lower; }
};

// Wilson score interval for independent Bernoulli trials.
[[nodiscard]] inline Interval wilson(std::uint64_t successes,
                                     std::uint64_t trials,
                                     double z = z_95) {
    if (trials == 0 || successes > trials) {
        throw std::invalid_argument("require 0 <= successes <= trials and trials > 0");
    }
    if (!std::isfinite(z) || z <= 0.0 || !std::isfinite(z * z)) {
        throw std::invalid_argument("z must be finite, positive and not too large");
    }
    const auto n = static_cast<double>(trials);
    const auto p = static_cast<double>(successes) / n;
    const auto squared_z = z * z;
    const auto denominator = 1.0 + squared_z / n;
    const auto midpoint = (p + squared_z / (2.0 * n)) / denominator;
    const auto margin = z * std::sqrt(
        (p * (1.0 - p) + squared_z / (4.0 * n)) / n) / denominator;
    return {p, std::clamp(midpoint - margin, 0.0, 1.0),
            std::clamp(midpoint + margin, 0.0, 1.0), successes, trials};
}

// Descriptive difference only; NOT an A/B test or statistical significance.
[[nodiscard]] inline double rate_difference(const Interval& a,
                                             const Interval& b) noexcept {
    return a.rate - b.rate;
}
} // namespace statistics

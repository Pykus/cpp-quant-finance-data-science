#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace statistics10 {

// Type-7 empirical quantile, widely used for descriptive statistics.
// Copies its input so callers never lose their original sample order.
[[nodiscard]] inline double quantile(std::vector<double> samples, double probability) {
    if (samples.empty()) throw std::invalid_argument("Samples cannot be empty");
    if (!std::isfinite(probability) || probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("Probability must be between zero and one");
    }
    for (const double value : samples) {
        if (!std::isfinite(value)) throw std::invalid_argument("Samples must be finite");
    }
    std::sort(samples.begin(), samples.end());
    const double position = probability * static_cast<double>(samples.size() - 1);
    const auto low = static_cast<std::size_t>(std::floor(position));
    const auto high = static_cast<std::size_t>(std::ceil(position));
    const double weight = position - static_cast<double>(low);
    const double result = samples[low] * (1.0 - weight) + samples[high] * weight;
    if (!std::isfinite(result)) throw std::overflow_error("Interpolated quantile overflow");
    return result;
}

[[nodiscard]] inline double interquartile_range(const std::vector<double>& samples) {
    const double low = quantile(samples, 0.25);
    const double high = quantile(samples, 0.75);
    const double result = high - low;
    if (!std::isfinite(result)) throw std::overflow_error("Interquartile range overflow");
    return result;
}

}  // namespace statistics10

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

namespace statistics {

struct OutlierReport {
    double median;
    double median_absolute_deviation;
    std::vector<std::optional<double>> modified_z_scores;
    std::vector<std::size_t> flagged_indices; // 0-based position in input
    bool scores_defined;
};

[[nodiscard]] inline double median_of(std::vector<double> values) {
    if (values.empty()) {
        throw std::invalid_argument("Median requires at least one observation");
    }
    for (double value : values) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Observations must be finite");
        }
    }
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;
    if (values.size() % 2 == 1) {
        return values[middle];
    }
    // Avoid possible overflow in (a+b)/2 when a and b have the same sign.
    return values[middle - 1] / 2.0 + values[middle] / 2.0;
}

// 0.67448975 scales MAD scores to the familiar standard-deviation units
// under a normal distribution. This is a descriptive screening heuristic.
[[nodiscard]] inline OutlierReport detect_outliers(
    std::span<const double> observations, double threshold = 3.5) {

    if (observations.size() < 3) {
        throw std::invalid_argument("At least three observations are needed");
    }
    if (!std::isfinite(threshold) || threshold <= 0.0) {
        throw std::invalid_argument("Threshold must be finite and positive");
    }
    const double center = median_of(std::vector<double>(
        observations.begin(), observations.end()));

    std::vector<double> distances;
    distances.reserve(observations.size());
    for (const double value : observations) {
        const double deviation = std::abs(value - center);
        if (!std::isfinite(deviation)) {
            throw std::overflow_error("A deviation overflowed");
        }
        distances.push_back(deviation);
    }
    const double mad = median_of(std::move(distances));

    OutlierReport result{center, mad, {}, {}, mad > 0.0};
    result.modified_z_scores.reserve(observations.size());
    if (!result.scores_defined) {
        result.modified_z_scores.resize(observations.size(), std::nullopt);
        return result; // All scores undefined; never silently divide by zero.
    }

    for (std::size_t i = 0; i < observations.size(); ++i) {
        const double score = 0.67448975 * ((observations[i] - center) / mad);
        if (!std::isfinite(score)) {
            throw std::overflow_error("A modified z-score overflowed");
        }
        result.modified_z_scores.push_back(score);
        if (std::abs(score) > threshold) {
            result.flagged_indices.push_back(i);
        }
    }
    return result;
}

} // namespace statistics

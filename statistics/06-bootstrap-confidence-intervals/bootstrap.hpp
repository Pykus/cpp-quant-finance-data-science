#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace stats {

enum class Statistic { mean, median };

struct Interval {
    double point_estimate = 0.0;
    double lower = 0.0;
    double upper = 0.0;
    double confidence = 0.0;
    std::size_t replicates = 0;
};

[[nodiscard]] inline double mean(const std::vector<double>& data) {
    if (data.empty()) {
        throw std::invalid_argument("mean requires observations");
    }
    const double result = std::accumulate(data.begin(), data.end(), 0.0)
                        / static_cast<double>(data.size());
    if (!std::isfinite(result)) {
        throw std::overflow_error("mean overflowed");
    }
    return result;
}

[[nodiscard]] inline double median(std::vector<double> data) {
    if (data.empty()) {
        throw std::invalid_argument("median requires observations");
    }
    std::sort(data.begin(), data.end());
    const std::size_t middle = data.size() / 2;
    return data.size() % 2 == 1
        ? data[middle]
        : data[middle - 1] / 2.0 + data[middle] / 2.0;
}

// The sorted vector is passed by reference to avoid an extra copy.
[[nodiscard]] inline double percentile(const std::vector<double>& sorted,
                                      double probability) {
    if (sorted.empty() || !std::isfinite(probability)
            || probability < 0.0 || probability > 1.0
            || !std::is_sorted(sorted.begin(), sorted.end())) {
        throw std::invalid_argument("percentile needs sorted data and p in [0,1]");
    }
    const double position = probability * static_cast<double>(sorted.size() - 1);
    const auto lower = static_cast<std::size_t>(position);
    const auto upper = std::min(lower + 1, sorted.size() - 1);
    const double weight = position - static_cast<double>(lower);
    return sorted[lower] * (1.0 - weight) + sorted[upper] * weight;
}

[[nodiscard]] inline Interval bootstrap_interval(
    const std::vector<double>& data,
    Statistic selected,
    std::size_t replicates = 2000,
    double confidence = 0.95,
    std::uint64_t seed = 42) {

    if (data.size() < 2) {
        throw std::invalid_argument("bootstrap requires at least two observations");
    }
    if (!std::all_of(data.begin(), data.end(),
                     [](double x) { return std::isfinite(x); })) {
        throw std::invalid_argument("observations must all be finite");
    }
    if (replicates < 100 || !std::isfinite(confidence)
            || confidence <= 0.0 || confidence >= 1.0) {
        throw std::invalid_argument("use >=100 replicates and 0<confidence<1");
    }
    if (data.size() > std::numeric_limits<std::size_t>::max() / replicates) {
        throw std::length_error("excessively large bootstrap request");
    }

    const auto estimate = [selected](const std::vector<double>& values) {
        switch (selected) {
            case Statistic::mean: return mean(values);
            case Statistic::median: return median(values);
        }
        throw std::invalid_argument("unknown statistic");
    };

    const double point = estimate(data);
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<std::size_t> sample_index(0, data.size() - 1);
    std::vector<double> resample;
    resample.reserve(data.size());
    std::vector<double> estimates;
    estimates.reserve(replicates);

    for (std::size_t b = 0; b < replicates; ++b) {
        resample.clear();
        for (std::size_t i = 0; i < data.size(); ++i) {
            resample.push_back(data[sample_index(engine)]);
        }
        estimates.push_back(estimate(resample));
    }
    std::sort(estimates.begin(), estimates.end());
    const double tail = (1.0 - confidence) / 2.0;
    return {point, percentile(estimates, tail),
            percentile(estimates, 1.0 - tail), confidence, replicates};
}

}  // namespace stats

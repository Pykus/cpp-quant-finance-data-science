#include "bootstrap_mean.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace statistics {
namespace {
[[nodiscard]] double checked_mean(const std::vector<double>& data) {
    long double sum = 0.0L;
    for (const double x : data) sum += static_cast<long double>(x);
    const long double value = sum / static_cast<long double>(data.size());
    if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<double>::max())
        throw std::overflow_error("mean cannot be represented");
    return static_cast<double>(value);
}
[[nodiscard]] double percentile(const std::vector<double>& sorted, double p) {
    const double position = (static_cast<double>(sorted.size()) - 1.0) * p;
    const auto lo = static_cast<std::size_t>(position);
    const auto hi = std::min(lo + 1, sorted.size() - 1);
    const long double weight = position - static_cast<double>(lo);
    return static_cast<double>((1.0L-weight)*sorted[lo] + weight*sorted[hi]);
}
} // namespace

BootstrapInterval bootstrap_mean_interval(
    const std::vector<double>& observations, std::size_t resamples,
    double confidence_level, std::uint64_t seed) {
    if (observations.size() < 2)
        throw std::invalid_argument("at least two observations required");
    if (resamples < 100 || resamples > 10'000'000)
        throw std::invalid_argument("resamples must be from 100 to 10000000");
    if (!std::isfinite(confidence_level) || confidence_level <= 0.0 || confidence_level >= 1.0)
        throw std::invalid_argument("confidence must be strictly between zero and one");
    for (double value : observations)
        if (!std::isfinite(value)) throw std::invalid_argument("non-finite observation");

    const double original_mean = checked_mean(observations);
    std::mt19937_64 engine(seed);
    std::uniform_int_distribution<std::size_t> pick(0, observations.size()-1);
    std::vector<double> means;
    means.reserve(resamples);
    long double running_mean = 0.0L;
    long double sum_squared_deviations = 0.0L;
    for (std::size_t b = 0; b < resamples; ++b) {
        long double sum = 0.0L;
        for (std::size_t i = 0; i < observations.size(); ++i)
            sum += static_cast<long double>(observations[pick(engine)]);
        const long double sample_mean = sum / static_cast<long double>(observations.size());
        if (!std::isfinite(sample_mean) ||
            std::abs(sample_mean) > std::numeric_limits<double>::max())
            throw std::overflow_error("resampled mean cannot be represented");
        means.push_back(static_cast<double>(sample_mean));
        const long double delta = sample_mean - running_mean;
        running_mean += delta / static_cast<long double>(b+1);
        sum_squared_deviations += delta * (sample_mean - running_mean);
    }
    const double se = static_cast<double>(
        std::sqrt(sum_squared_deviations / static_cast<long double>(resamples-1)));
    std::sort(means.begin(), means.end());
    const double tail = (1.0 - confidence_level) / 2.0;
    return {original_mean, percentile(means, tail), percentile(means, 1.0-tail),
            se, resamples};
}
} // namespace statistics

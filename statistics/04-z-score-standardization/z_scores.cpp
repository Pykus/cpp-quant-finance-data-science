#include "z_scores.hpp"

#include <cmath>
#include <stdexcept>

namespace statistics {

SampleSummary describe_sample(const std::vector<double>& observations) {
    if (observations.size() < 2) {
        throw std::invalid_argument("At least two observations are required");
    }

    long double sum = 0.0L;
    for (const double value : observations) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Observations must be finite numbers");
        }
        sum += static_cast<long double>(value);
    }
    const long double count = static_cast<long double>(observations.size());
    const long double mean = sum / count;

    long double sum_of_squared_deviations = 0.0L;
    for (const double value : observations) {
        const long double difference = static_cast<long double>(value) - mean;
        sum_of_squared_deviations += difference * difference;
    }

    // We are estimating the spread of a sample, so divide by (n - 1).
    const long double variance = sum_of_squared_deviations / (count - 1.0L);
    const long double standard_deviation = std::sqrt(variance);

    if (!(standard_deviation > 0.0L) ||
        !std::isfinite(standard_deviation) ||
        !std::isfinite(static_cast<double>(mean)) ||
        !std::isfinite(static_cast<double>(standard_deviation)) ||
        !(static_cast<double>(standard_deviation) > 0.0)) {
        throw std::domain_error("Cannot standardize a constant or unrepresentable sample");
    }

    return {static_cast<double>(mean), static_cast<double>(standard_deviation)};
}

std::vector<double> z_scores(const std::vector<double>& observations) {
    const SampleSummary summary = describe_sample(observations);

    std::vector<double> standardized;
    standardized.reserve(observations.size());
    for (const double value : observations) {
        const long double centered = static_cast<long double>(value) -
                                     static_cast<long double>(summary.mean);
        standardized.push_back(static_cast<double>(
            centered / static_cast<long double>(summary.sample_standard_deviation)));
    }
    return standardized;
}

}  // namespace statistics

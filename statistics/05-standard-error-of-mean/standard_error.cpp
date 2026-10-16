#include "standard_error.hpp"

#include <cmath>
#include <stdexcept>

namespace statistics {

MeanUncertainty summarize_mean(const std::vector<double>& sample) {
    if (sample.size() < 2) {
        throw std::invalid_argument("A sample needs at least two observations");
    }

    // Welford's one-pass update avoids subtracting two very large sums.
    std::size_t count = 0;
    double mean = 0.0;
    double squared_deviation_sum = 0.0;
    for (const double value : sample) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Every observation must be finite");
        }
        ++count;
        const double delta = value - mean;
        mean += delta / static_cast<double>(count);
        const double updated_delta = value - mean;
        squared_deviation_sum += delta * updated_delta;
    }

    const double variance = squared_deviation_sum /
                            static_cast<double>(sample.size() - 1);
    if (!std::isfinite(mean) || !std::isfinite(variance) || variance < 0.0) {
        throw std::overflow_error("The sample is outside supported numeric range");
    }

    const double standard_deviation = std::sqrt(variance);
    const double standard_error =
        standard_deviation / std::sqrt(static_cast<double>(sample.size()));
    if (!std::isfinite(standard_error)) {
        throw std::overflow_error("Standard error is outside supported numeric range");
    }

    return {sample.size(), mean, standard_deviation, standard_error};
}

}  // namespace statistics

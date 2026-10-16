#pragma once

#include <cstddef>
#include <vector>

namespace statistics {

struct MeanUncertainty {
    std::size_t sample_size;
    double mean;
    double sample_standard_deviation;
    double standard_error;
};

// Estimates the spread of the sample mean from independent observations.
// Requires at least two finite observations; throws on invalid inputs or overflow.
[[nodiscard]] MeanUncertainty summarize_mean(const std::vector<double>& sample);

}  // namespace statistics

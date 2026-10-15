#pragma once

#include <vector>

namespace statistics {

struct SampleSummary {
    double mean;
    double sample_standard_deviation;
};

// Reject fewer than two observations, non-finite values, and zero spread.
[[nodiscard]] SampleSummary describe_sample(const std::vector<double>& observations);

// Each element is (value - sample mean) / sample standard deviation.
[[nodiscard]] std::vector<double> z_scores(const std::vector<double>& observations);

}  // namespace statistics

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace statistics {
struct BootstrapInterval {
    double sample_mean{};
    double lower{};
    double upper{};
    double bootstrap_standard_error{};
    std::size_t resamples{};
};
[[nodiscard]] BootstrapInterval bootstrap_mean_interval(
    const std::vector<double>& observations,
    std::size_t resamples,
    double confidence_level,
    std::uint64_t seed = 20261017);
} // namespace statistics

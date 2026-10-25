#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace systems10 {

struct Summary {
    std::size_t observations;
    std::uint64_t minimum_ns;
    std::uint64_t median_ns;
    std::uint64_t p99_ns;
    std::uint64_t p999_ns;
    std::uint64_t maximum_ns;
};

// Exact percentiles require recording every observation. For bounded
// production telemetry, choose a histogram or a streaming sketch.
class LatencyRecorder {
public:
    void add(std::chrono::nanoseconds elapsed) {
        if (elapsed.count() < 0) {
            throw std::invalid_argument("Latency must not be negative");
        }
        values_.push_back(static_cast<std::uint64_t>(elapsed.count()));
    }

    [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }

    [[nodiscard]] Summary summarize() const {
        if (values_.empty()) throw std::domain_error("No latency observations");
        std::vector<std::uint64_t> sorted = values_;
        std::sort(sorted.begin(), sorted.end());
        const auto at = [&](double percentile) {
            // Nearest-rank percentile: rank=ceil(n*p), counted from one.
            const auto rank = static_cast<std::size_t>(
                std::ceil(static_cast<double>(sorted.size()) * percentile));
            return sorted[rank - 1];
        };
        return {
            sorted.size(), sorted.front(), at(.50), at(.99), at(.999), sorted.back()
        };
    }

private:
    std::vector<std::uint64_t> values_;
};

}  // namespace systems10

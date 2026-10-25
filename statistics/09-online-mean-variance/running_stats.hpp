#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace statistics09 {

// Welford's algorithm: stable running mean and variance, constant memory.
class RunningStats {
public:
    void add(double value) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("Observations must be finite");
        }
        if (count_ == std::numeric_limits<std::size_t>::max()) {
            throw std::overflow_error("Observation count overflow");
        }
        const std::size_t next_count = count_ + 1;
        const double difference = value - mean_;
        const double next_mean = mean_ + difference / static_cast<double>(next_count);
        const double next_m2 = m2_ + difference * (value - next_mean);
        if (!std::isfinite(next_mean) || !std::isfinite(next_m2)) {
            throw std::overflow_error("Running moment overflow");
        }
        count_ = next_count;
        mean_ = next_mean;
        m2_ = next_m2;
    }

    // Combining independent batches is useful for chunked data processing.
    void merge(const RunningStats& other) {
        if (other.count_ == 0) return;
        if (count_ == 0) {
            *this = other;
            return;
        }
        if (other.count_ > std::numeric_limits<std::size_t>::max() - count_) {
            throw std::overflow_error("Combined observation count overflow");
        }
        const auto total = count_ + other.count_;
        const double n1 = static_cast<double>(count_);
        const double n2 = static_cast<double>(other.count_);
        const double nt = static_cast<double>(total);
        const double delta = other.mean_ - mean_;
        const double next_mean = mean_ + delta * (n2 / nt);
        const double next_m2 = m2_ + other.m2_ + delta * delta * (n1 * (n2 / nt));
        if (!std::isfinite(next_mean) || !std::isfinite(next_m2)) {
            throw std::overflow_error("Combined running moments overflow");
        }
        count_ = total;
        mean_ = next_mean;
        m2_ = next_m2;
    }

    [[nodiscard]] std::size_t count() const noexcept { return count_; }

    [[nodiscard]] double mean() const {
        if (count_ == 0) throw std::domain_error("Mean requires observations");
        return mean_;
    }

    [[nodiscard]] double population_variance() const {
        if (count_ == 0) throw std::domain_error("Variance requires observations");
        return std::max(0.0, m2_) / static_cast<double>(count_);
    }

    [[nodiscard]] double sample_variance() const {
        if (count_ < 2) throw std::domain_error("Sample variance needs two observations");
        return std::max(0.0, m2_) / static_cast<double>(count_ - 1);
    }

private:
    std::size_t count_ = 0;
    double mean_ = 0.0;
    double m2_ = 0.0;  // Sum of squared deviations from the running mean.
};

}  // namespace statistics09

#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

namespace stats {
// One-pass Welford estimator with exact-count merge of independent summaries.
class RunningMoments {
public:
    void add(double x) {
        if (!std::isfinite(x)) throw std::invalid_argument("observation is not finite");
        if (count_ == std::numeric_limits<std::uint64_t>::max())
            throw std::overflow_error("too many observations");
        const long double delta = static_cast<long double>(x) - mean_;
        ++count_;
        mean_ += delta / static_cast<long double>(count_);
        m2_ += delta * (static_cast<long double>(x) - mean_);
        min_ = std::min(min_, x);
        max_ = std::max(max_, x);
    }

    void merge(const RunningMoments& rhs) {
        if (!rhs.count_) return;
        if (!count_) { *this = rhs; return; }
        if (rhs.count_ > std::numeric_limits<std::uint64_t>::max() - count_)
            throw std::overflow_error("merged observation count overflow");
        const long double n1 = static_cast<long double>(count_);
        const long double n2 = static_cast<long double>(rhs.count_);
        const long double n = n1 + n2;
        const long double delta = rhs.mean_ - mean_;
        const long double mean = mean_ + delta * n2 / n;
        const long double m2 = m2_ + rhs.m2_ + delta * delta * (n1 * n2 / n);
        count_ += rhs.count_;
        mean_ = mean;
        m2_ = m2;
        min_ = std::min(min_, rhs.min_);
        max_ = std::max(max_, rhs.max_);
    }

    [[nodiscard]] std::uint64_t count() const noexcept { return count_; }
    [[nodiscard]] std::optional<double> mean() const noexcept {
        if (!count_) return std::nullopt;
        return static_cast<double>(mean_);
    }
    [[nodiscard]] std::optional<double> population_variance() const noexcept {
        if (!count_) return std::nullopt;
        return static_cast<double>(std::max(0.0L, m2_) / count_);
    }
    [[nodiscard]] std::optional<double> sample_variance() const noexcept {
        if (count_ < 2) return std::nullopt;
        return static_cast<double>(std::max(0.0L, m2_) / (count_ - 1));
    }
    [[nodiscard]] std::optional<double> sample_stddev() const noexcept {
        auto v = sample_variance();
        if (!v) return std::nullopt;
        return std::sqrt(*v);
    }
    [[nodiscard]] std::optional<double> minimum() const noexcept {
        if (!count_) return std::nullopt;
        return min_;
    }
    [[nodiscard]] std::optional<double> maximum() const noexcept {
        if (!count_) return std::nullopt;
        return max_;
    }

private:
    std::uint64_t count_{0};
    long double mean_{0};
    long double m2_{0};
    double min_{std::numeric_limits<double>::infinity()};
    double max_{-std::numeric_limits<double>::infinity()};
};
}  // namespace stats

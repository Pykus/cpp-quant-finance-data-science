#pragma once
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>

namespace statistics {
class OnlineCovariance {
public:
    [[nodiscard]] std::size_t count() const noexcept { return n_; }
    [[nodiscard]] double mean_x() const noexcept { return mean_x_; }
    [[nodiscard]] double mean_y() const noexcept { return mean_y_; }

    void add(double x, double y) {
        if (!std::isfinite(x) || !std::isfinite(y))
            throw std::invalid_argument("Observations must be finite");
        if (n_ == std::numeric_limits<std::size_t>::max())
            throw std::overflow_error("Observation count overflow");
        const std::size_t count = n_ + 1;
        const double weight = 1.0 / static_cast<double>(count);
        const double dx = x - mean_x_;
        const double dy = y - mean_y_;
        const double mx = mean_x_ + dx * weight;
        const double my = mean_y_ + dy * weight;
        const double xx = m2_x_ + dx * (x - mx);
        const double yy = m2_y_ + dy * (y - my);
        const double xy = co_moment_ + dx * (y - my);
        if (!std::isfinite(mx) || !std::isfinite(my) ||
            !std::isfinite(xx) || !std::isfinite(yy) || !std::isfinite(xy))
            throw std::overflow_error("Statistics exceeded floating-point range");
        n_ = count; mean_x_ = mx; mean_y_ = my;
        m2_x_ = xx; m2_y_ = yy; co_moment_ = xy;
    }

    void merge(const OnlineCovariance& other) {
        if (other.n_ == 0) return;
        if (n_ == 0) { *this = other; return; }
        if (other.n_ > std::numeric_limits<std::size_t>::max() - n_)
            throw std::overflow_error("Observation count overflow");
        const std::size_t total = n_ + other.n_;
        const double a = static_cast<double>(n_);
        const double b = static_cast<double>(other.n_);
        const double size = static_cast<double>(total);
        const double dx = other.mean_x_ - mean_x_;
        const double dy = other.mean_y_ - mean_y_;
        const double factor = (a / size) * b;
        const double mx = mean_x_ + dx * (b / size);
        const double my = mean_y_ + dy * (b / size);
        const double xx = m2_x_ + other.m2_x_ + dx * dx * factor;
        const double yy = m2_y_ + other.m2_y_ + dy * dy * factor;
        const double xy = co_moment_ + other.co_moment_ + dx * dy * factor;
        if (!std::isfinite(mx) || !std::isfinite(my) ||
            !std::isfinite(xx) || !std::isfinite(yy) || !std::isfinite(xy))
            throw std::overflow_error("Merged statistics exceeded floating-point range");
        n_ = total; mean_x_ = mx; mean_y_ = my;
        m2_x_ = xx; m2_y_ = yy; co_moment_ = xy;
    }

    [[nodiscard]] std::optional<double> sample_covariance() const noexcept {
        if (n_ < 2) return std::nullopt;
        return co_moment_ / static_cast<double>(n_ - 1);
    }
    [[nodiscard]] std::optional<double> population_covariance() const noexcept {
        if (n_ == 0) return std::nullopt;
        return co_moment_ / static_cast<double>(n_);
    }
    [[nodiscard]] std::optional<double> variance_x() const noexcept {
        if (n_ < 2) return std::nullopt;
        return m2_x_ / static_cast<double>(n_ - 1);
    }
    [[nodiscard]] std::optional<double> variance_y() const noexcept {
        if (n_ < 2) return std::nullopt;
        return m2_y_ / static_cast<double>(n_ - 1);
    }
    [[nodiscard]] std::optional<double> correlation() const noexcept {
        if (n_ < 2 || m2_x_ <= 0.0 || m2_y_ <= 0.0) return std::nullopt;
        const double scale = std::sqrt(m2_x_) * std::sqrt(m2_y_);
        if (!(scale > 0.0) || !std::isfinite(scale)) return std::nullopt;
        const double value = co_moment_ / scale;
        if (!std::isfinite(value)) return std::nullopt;
        return std::fmax(-1.0, std::fmin(1.0, value));
    }
private:
    std::size_t n_ = 0;
    double mean_x_ = 0.0, mean_y_ = 0.0;
    double m2_x_ = 0.0, m2_y_ = 0.0, co_moment_ = 0.0;
};
} // namespace statistics

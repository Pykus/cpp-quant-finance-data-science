#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace statistics {

struct RankedValues {
    std::vector<long double> ranks;
    std::size_t tied_observations{};
};

struct SpearmanResult {
    double rho{};
    std::size_t pairs{};
    std::size_t tied_x{};
    std::size_t tied_y{};
};

// Midranks preserve the usual Spearman definition when observations are tied.
[[nodiscard]] inline RankedValues midranks(const std::vector<double>& values) {
    std::vector<std::size_t> order(values.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    for (const double value : values) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("All observations must be finite");
        }
    }
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return values[a] < values[b];
    });

    RankedValues result{std::vector<long double>(values.size()), 0};
    for (std::size_t i = 0; i < order.size();) {
        std::size_t end = i + 1;
        while (end < order.size() && values[order[i]] == values[order[end]]) {
            ++end;
        }
        // Sorted zero-based positions [i, end) correspond to ranks [i+1, end].
        const auto rank = (static_cast<long double>(i) +
                           static_cast<long double>(end) + 1.0L) / 2.0L;
        for (std::size_t k = i; k < end; ++k) {
            result.ranks[order[k]] = rank;
        }
        if (end - i > 1) {
            result.tied_observations += end - i;
        }
        i = end;
    }
    return result;
}

[[nodiscard]] inline SpearmanResult spearman(const std::vector<double>& x,
                                             const std::vector<double>& y) {
    if (x.size() != y.size()) {
        throw std::invalid_argument("Paired series must have the same length");
    }
    if (x.size() < 2) {
        throw std::invalid_argument("At least two pairs are required");
    }
    const auto rx = midranks(x);
    const auto ry = midranks(y);
    const long double mean_rank = (static_cast<long double>(x.size()) + 1.0L) / 2.0L;
    long double cross = 0.0L;
    long double xx = 0.0L;
    long double yy = 0.0L;
    for (std::size_t i = 0; i < x.size(); ++i) {
        const auto a = rx.ranks[i] - mean_rank;
        const auto b = ry.ranks[i] - mean_rank;
        cross += a * b;
        xx += a * a;
        yy += b * b;
    }
    if (xx == 0.0L || yy == 0.0L) {
        throw std::domain_error("Spearman correlation is undefined for a constant series");
    }
    const auto correlation = cross / (std::sqrt(xx) * std::sqrt(yy));
    // Bound possible floating-point error near exactly +1/-1.
    const auto bounded = std::clamp(correlation, -1.0L, 1.0L);
    return {static_cast<double>(bounded), x.size(), rx.tied_observations,
            ry.tied_observations};
}

}  // namespace statistics

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>

namespace quant::antithetic {

struct EuropeanCall {
    double spot;
    double strike;
    double rate;
    double volatility;
    double maturity;
};

enum class Method { independent, antithetic };

struct Estimate {
    double price;
    double variance;
    double standard_error;
    double low95;
    double high95;
    std::size_t observations;
    std::size_t payoffs;
};

inline void validate(const EuropeanCall& o) {
    if (!std::isfinite(o.spot) || o.spot <= 0.0 ||
        !std::isfinite(o.strike) || o.strike <= 0.0 ||
        !std::isfinite(o.rate) ||
        !std::isfinite(o.volatility) || o.volatility <= 0.0 ||
        !std::isfinite(o.maturity) || o.maturity <= 0.0) {
        throw std::invalid_argument("invalid European call parameters");
    }
}

[[nodiscard]] inline double black_scholes(const EuropeanCall& o) {
    validate(o);
    const double scaled = o.volatility * std::sqrt(o.maturity);
    const double d1 = (std::log(o.spot / o.strike) +
        (o.rate + 0.5 * o.volatility * o.volatility) * o.maturity) / scaled;
    const double d2 = d1 - scaled;
    const auto cdf = [](double z) { return 0.5 * std::erfc(-z / std::sqrt(2.0)); };
    const double price = o.spot * cdf(d1) -
        o.strike * std::exp(-o.rate * o.maturity) * cdf(d2);
    if (!std::isfinite(price)) throw std::overflow_error("analytic price overflow");
    return price;
}

struct RunningMoments {
    std::size_t count = 0;
    double mean = 0.0;
    double squared = 0.0;

    void add(double x) {
        if (!std::isfinite(x)) throw std::overflow_error("non-finite payoff");
        ++count;
        const double delta = x - mean;
        mean += delta / static_cast<double>(count);
        squared += delta * (x - mean);
    }
};

[[nodiscard]] inline Estimate estimate(const EuropeanCall& o,
                                       std::size_t pairs,
                                       std::uint64_t seed,
                                       Method method) {
    validate(o);
    if (pairs < 2 || pairs > std::numeric_limits<std::size_t>::max() / 2) {
        throw std::invalid_argument("at least two pairs required, without count overflow");
    }
    if (method != Method::independent && method != Method::antithetic) {
        throw std::invalid_argument("unknown sampling method");
    }

    const double drift = (o.rate - 0.5 * o.volatility * o.volatility) * o.maturity;
    const double diffusion = o.volatility * std::sqrt(o.maturity);
    const double discount = std::exp(-o.rate * o.maturity);
    if (!std::isfinite(drift) || !std::isfinite(diffusion) ||
        !std::isfinite(discount)) throw std::overflow_error("model parameter overflow");

    const auto payoff = [&](double z) {
        const double terminal = o.spot * std::exp(drift + diffusion * z);
        const double result = discount * std::max(terminal - o.strike, 0.0);
        if (!std::isfinite(result)) throw std::overflow_error("payoff overflow");
        return result;
    };

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> normal(0.0, 1.0);
    RunningMoments stats;

    for (std::size_t i = 0; i < pairs; ++i) {
        const double z = normal(rng);
        const double a = payoff(z);
        if (method == Method::antithetic) {
            // Each correlated pair mean counts as ONE independent observation.
            const double b = payoff(-z);
            stats.add(0.5 * a + 0.5 * b);
        } else {
            stats.add(a);
            stats.add(payoff(normal(rng)));
        }
    }

    const double sample_variance = stats.squared /
        static_cast<double>(stats.count - 1);
    const double se = std::sqrt(sample_variance / static_cast<double>(stats.count));
    if (!std::isfinite(se) || !std::isfinite(sample_variance)) {
        throw std::overflow_error("statistics overflow");
    }
    constexpr double z95 = 1.959963984540054;
    return {stats.mean, sample_variance, se,
            stats.mean - z95 * se, stats.mean + z95 * se,
            stats.count, 2 * pairs};
}

} // namespace quant::antithetic

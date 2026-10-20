#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numbers>
#include <numeric>
#include <stdexcept>

namespace quant {

enum class OptionKind { call, put };

struct OptionInputs {
    double spot;
    double strike;
    double rate;
    double dividend_yield;
    double years_to_expiry;
};

struct PriceBounds {
    double lower;
    double upper;
};

struct ImpliedVolatilityResult {
    double volatility;
    std::size_t iterations;
    double absolute_repricing_error;
};

namespace detail {

struct DiscountedValues {
    double asset;
    double strike;
};

[[nodiscard]] inline DiscountedValues discounted_values(const OptionInputs& inputs) {
    if (!std::isfinite(inputs.spot) || inputs.spot <= 0.0 ||
        !std::isfinite(inputs.strike) || inputs.strike <= 0.0 ||
        !std::isfinite(inputs.rate) ||
        !std::isfinite(inputs.dividend_yield) ||
        !std::isfinite(inputs.years_to_expiry) || inputs.years_to_expiry <= 0.0) {
        throw std::invalid_argument("Spot, strike and expiry must be positive and all inputs finite");
    }

    const double asset = inputs.spot * std::exp(-inputs.dividend_yield * inputs.years_to_expiry);
    const double strike = inputs.strike * std::exp(-inputs.rate * inputs.years_to_expiry);
    if (!std::isfinite(asset) || !std::isfinite(strike) || asset <= 0.0 || strike <= 0.0) {
        throw std::invalid_argument("Discounted asset and strike must be positive and finite");
    }
    return {asset, strike};
}

[[nodiscard]] inline double normal_cdf(double x) noexcept {
    return 0.5 * std::erfc(-x / std::numbers::sqrt2);
}

[[nodiscard]] inline double normal_pdf(double x) noexcept {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * std::numbers::pi);
}

[[nodiscard]] inline PriceBounds bounds(OptionKind kind, const DiscountedValues& pv) noexcept {
    if (kind == OptionKind::call) {
        return {std::max(0.0, pv.asset - pv.strike), pv.asset};
    }
    return {std::max(0.0, pv.strike - pv.asset), pv.strike};
}

}  // namespace detail

[[nodiscard]] inline PriceBounds arbitrage_bounds(OptionKind kind, const OptionInputs& inputs) {
    return detail::bounds(kind, detail::discounted_values(inputs));
}

[[nodiscard]] inline double black_scholes_price(
    OptionKind kind, const OptionInputs& inputs, double volatility) {
    const auto pv = detail::discounted_values(inputs);
    if (!std::isfinite(volatility) || volatility < 0.0) {
        throw std::invalid_argument("Volatility must be nonnegative and finite");
    }
    const auto limits = detail::bounds(kind, pv);
    if (volatility == 0.0) {
        return limits.lower;
    }

    const double root_time = std::sqrt(inputs.years_to_expiry);
    const double scaled_volatility = volatility * root_time;
    const double log_moneyness = std::log(inputs.spot) - std::log(inputs.strike);
    const double d1 = (log_moneyness +
        (inputs.rate - inputs.dividend_yield + 0.5 * volatility * volatility)
            * inputs.years_to_expiry) / scaled_volatility;
    const double d2 = d1 - scaled_volatility;

    const double value = kind == OptionKind::call
        ? pv.asset * detail::normal_cdf(d1) - pv.strike * detail::normal_cdf(d2)
        : pv.strike * detail::normal_cdf(-d2) - pv.asset * detail::normal_cdf(-d1);
    if (!std::isfinite(value)) {
        throw std::invalid_argument("Model price could not be represented");
    }
    return std::clamp(value, limits.lower, limits.upper);
}

[[nodiscard]] inline double black_scholes_vega(const OptionInputs& inputs, double volatility) {
    const auto pv = detail::discounted_values(inputs);
    if (!std::isfinite(volatility) || volatility <= 0.0) {
        throw std::invalid_argument("Vega requires positive finite volatility");
    }
    const double root_time = std::sqrt(inputs.years_to_expiry);
    const double d1 = ((std::log(inputs.spot) - std::log(inputs.strike)) +
        (inputs.rate - inputs.dividend_yield + 0.5 * volatility * volatility)
            * inputs.years_to_expiry) / (volatility * root_time);
    const double result = pv.asset * root_time * detail::normal_pdf(d1);
    if (!std::isfinite(result)) {
        throw std::invalid_argument("Model vega could not be represented");
    }
    return result;
}

// A bracketed solver does not need an initial guess and never divides by vega.
[[nodiscard]] inline ImpliedVolatilityResult implied_volatility(
    OptionKind kind, const OptionInputs& inputs, double market_price,
    double absolute_price_tolerance = 1e-10, std::size_t max_iterations = 200) {
    const auto limits = arbitrage_bounds(kind, inputs);
    if (!std::isfinite(market_price) || market_price < limits.lower ||
        market_price >= limits.upper) {
        throw std::invalid_argument("Market price must lie in [lower, upper) arbitrage bounds");
    }
    if (!std::isfinite(absolute_price_tolerance) || absolute_price_tolerance <= 0.0 ||
        max_iterations == 0) {
        throw std::invalid_argument("Tolerance and iteration count must be positive");
    }
    if (market_price == limits.lower) {
        return {0.0, 0, 0.0};
    }

    double low = 0.0;
    double high = 0.25;
    constexpr double maximum_volatility = 64.0;
    while (black_scholes_price(kind, inputs, high) < market_price &&
           high < maximum_volatility) {
        high *= 2.0;
    }
    if (black_scholes_price(kind, inputs, high) < market_price) {
        throw std::runtime_error("Unable to bracket the requested market price");
    }

    for (std::size_t iteration = 1; iteration <= max_iterations; ++iteration) {
        const double midpoint = std::midpoint(low, high);
        const double estimated_price = black_scholes_price(kind, inputs, midpoint);
        const double error = estimated_price - market_price;
        if (std::abs(error) <= absolute_price_tolerance) {
            return {midpoint, iteration, std::abs(error)};
        }
        if (midpoint == low || midpoint == high) {
            break;
        }
        if (error < 0.0) {
            low = midpoint;
        } else {
            high = midpoint;
        }
    }
    throw std::runtime_error("Bisection did not reach the requested price tolerance");
}

}  // namespace quant

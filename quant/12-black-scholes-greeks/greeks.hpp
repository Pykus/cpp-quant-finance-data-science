#pragma once

#include <cmath>
#include <stdexcept>

namespace quant {

enum class OptionKind { call, put };

struct OptionInputs {
    double spot;
    double strike;
    double rate;
    double dividend_yield;
    double volatility;
    double maturity_years;
};

struct OptionGreeks {
    double price;
    double delta;
    double gamma;
    double vega;
};

// Standard-normal distribution functions. erfc behaves well in both tails.
[[nodiscard]] inline double normal_cdf(double x) noexcept {
    constexpr double inverse_sqrt_two = 0.70710678118654752440;
    return 0.5 * std::erfc(-x * inverse_sqrt_two);
}

[[nodiscard]] inline double normal_pdf(double x) noexcept {
    constexpr double inverse_sqrt_two_pi = 0.39894228040143267794;
    return inverse_sqrt_two_pi * std::exp(-0.5 * x * x);
}

inline void validate(const OptionInputs& x) {
    if (!std::isfinite(x.spot) || !std::isfinite(x.strike) ||
        !std::isfinite(x.rate) || !std::isfinite(x.dividend_yield) ||
        !std::isfinite(x.volatility) || !std::isfinite(x.maturity_years) ||
        x.spot <= 0.0 || x.strike <= 0.0 || x.volatility <= 0.0 ||
        x.maturity_years <= 0.0) {
        throw std::invalid_argument(
            "spot, strike, volatility and maturity must be positive; "
            "all inputs must be finite");
    }
}

// European options, lognormal diffusion, continuous dividends and rates.
[[nodiscard]] inline OptionGreeks black_scholes(
    OptionKind kind, const OptionInputs& x) {
    validate(x);
    const double root_t = std::sqrt(x.maturity_years);
    const double sigma_root_t = x.volatility * root_t;
    const double d1 = (
        std::log(x.spot / x.strike) +
        (x.rate - x.dividend_yield +
         0.5 * x.volatility * x.volatility) * x.maturity_years
    ) / sigma_root_t;
    const double d2 = d1 - sigma_root_t;
    const double spot_discount = x.spot * std::exp(
        -x.dividend_yield * x.maturity_years);
    const double strike_discount = x.strike * std::exp(
        -x.rate * x.maturity_years);
    const double q_discount = std::exp(
        -x.dividend_yield * x.maturity_years);

    OptionGreeks result{};
    if (kind == OptionKind::call) {
        result.price = spot_discount * normal_cdf(d1) -
                       strike_discount * normal_cdf(d2);
        result.delta = q_discount * normal_cdf(d1);
    } else {
        result.price = strike_discount * normal_cdf(-d2) -
                       spot_discount * normal_cdf(-d1);
        result.delta = q_discount * (normal_cdf(d1) - 1.0);
    }
    result.gamma = q_discount * normal_pdf(d1) /
                   (x.spot * sigma_root_t);
    result.vega = spot_discount * normal_pdf(d1) * root_t;

    if (!std::isfinite(result.price) || !std::isfinite(result.delta) ||
        !std::isfinite(result.gamma) || !std::isfinite(result.vega)) {
        throw std::overflow_error("Black-Scholes computation overflowed");
    }
    return result;
}

// Central difference: (f(S+h)-f(S-h))/(2h).
[[nodiscard]] inline double finite_difference_delta(
    OptionKind kind, OptionInputs x, double spot_bump_fraction = 1e-4) {
    validate(x);
    if (!std::isfinite(spot_bump_fraction) ||
        spot_bump_fraction <= 0.0 || spot_bump_fraction >= 1.0) {
        throw std::invalid_argument("spot bump fraction must be in (0, 1)");
    }
    const double h = x.spot * spot_bump_fraction;
    x.spot += h;
    const double higher = black_scholes(kind, x).price;
    x.spot -= 2.0 * h;
    const double lower = black_scholes(kind, x).price;
    return (higher - lower) / (2.0 * h);
}

// Central second difference: (f(S+h)-2f(S)+f(S-h))/h^2.
[[nodiscard]] inline double finite_difference_gamma(
    OptionKind kind, OptionInputs x, double spot_bump_fraction = 1e-3) {
    validate(x);
    if (!std::isfinite(spot_bump_fraction) ||
        spot_bump_fraction <= 0.0 || spot_bump_fraction >= 1.0) {
        throw std::invalid_argument("spot bump fraction must be in (0, 1)");
    }
    const double center = black_scholes(kind, x).price;
    const double h = x.spot * spot_bump_fraction;
    x.spot += h;
    const double higher = black_scholes(kind, x).price;
    x.spot -= 2.0 * h;
    const double lower = black_scholes(kind, x).price;
    return (higher - 2.0 * center + lower) / (h * h);
}

// Central volatility bump. The bump is ABSOLUTE volatility, not percent.
[[nodiscard]] inline double finite_difference_vega(
    OptionKind kind, OptionInputs x, double volatility_bump = 1e-4) {
    validate(x);
    if (!std::isfinite(volatility_bump) || volatility_bump <= 0.0 ||
        volatility_bump >= x.volatility) {
        throw std::invalid_argument(
            "volatility bump must be positive and below volatility");
    }
    x.volatility += volatility_bump;
    const double higher = black_scholes(kind, x).price;
    x.volatility -= 2.0 * volatility_bump;
    const double lower = black_scholes(kind, x).price;
    return (higher - lower) / (2.0 * volatility_bump);
}

// A positive option position is long; a negative position is short.
// A multiplier of 100 means that one contract represents 100 units.
[[nodiscard]] inline double shares_for_zero_delta(
    double signed_option_contracts, double multiplier, double option_delta) {
    if (!std::isfinite(signed_option_contracts) ||
        !std::isfinite(multiplier) || !std::isfinite(option_delta) ||
        multiplier <= 0.0) {
        throw std::invalid_argument("invalid hedge inputs");
    }
    const double shares =
        -signed_option_contracts * multiplier * option_delta;
    if (!std::isfinite(shares)) {
        throw std::overflow_error("hedge shares overflowed");
    }
    return shares;
}

} // namespace quant

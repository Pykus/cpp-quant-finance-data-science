#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace quant::binomial {

enum class OptionKind { call, put };
enum class ExerciseStyle { european, american };

struct Contract {
    double spot;
    double strike;
    double maturity_years;
    double annual_rate;
    double annual_dividend_yield;
    double annual_volatility;
    std::size_t steps;
    OptionKind kind;
    ExerciseStyle exercise;
};

namespace detail {

inline void validate(const Contract& c) {
    if (!std::isfinite(c.spot) || !std::isfinite(c.strike) ||
        !std::isfinite(c.maturity_years) || !std::isfinite(c.annual_rate) ||
        !std::isfinite(c.annual_dividend_yield) || !std::isfinite(c.annual_volatility) ||
        c.spot <= 0.0 || c.strike <= 0.0 || c.maturity_years <= 0.0 ||
        c.annual_volatility < 0.0) {
        throw std::invalid_argument("Finite spot/strike/maturity must be positive; volatility nonnegative");
    }
    if (c.steps < 1 || c.steps > 5000) {
        throw std::invalid_argument("Tree steps must be between 1 and 5000");
    }
}

[[nodiscard]] inline double intrinsic(double spot, double strike, OptionKind kind) noexcept {
    return kind == OptionKind::call ? std::max(spot - strike, 0.0)
                                    : std::max(strike - spot, 0.0);
}

[[nodiscard]] inline double checked(double x) {
    if (!std::isfinite(x)) {
        throw std::overflow_error("Pricing calculation exceeded floating-point range");
    }
    return x;
}

[[nodiscard]] inline double safe_exp(double x) {
    return checked(std::exp(x));
}

[[nodiscard]] inline double deterministic_price(const Contract& c) {
    const double dt = c.maturity_years / static_cast<double>(c.steps);
    const std::size_t first = c.exercise == ExerciseStyle::american ? 0 : c.steps;
    double best = 0.0;
    for (std::size_t i = first; i <= c.steps; ++i) {
        const double t = static_cast<double>(i) * dt;
        const double stock = checked(c.spot *
            safe_exp((c.annual_rate - c.annual_dividend_yield) * t));
        const double discounted = checked(safe_exp(-c.annual_rate * t) *
            intrinsic(stock, c.strike, c.kind));
        best = std::max(best, discounted);
    }
    return best;
}

} // namespace detail

// Cox-Ross-Rubinstein model: O(steps^2) time, O(steps) storage.
[[nodiscard]] inline double price(const Contract& c) {
    detail::validate(c);
    if (c.annual_volatility == 0.0) {
        return detail::deterministic_price(c);
    }

    const double n = static_cast<double>(c.steps);
    const double dt = c.maturity_years / n;
    const double h = c.annual_volatility * std::sqrt(dt);
    const double drift = (c.annual_rate - c.annual_dividend_yield) * dt;
    // expm1 avoids cancellation when exp(x) is near one.
    const double p = std::expm1(drift + h) / std::expm1(2.0 * h);
    constexpr double tolerance = 1e-12;
    if (!std::isfinite(p) || p < -tolerance || p > 1.0 + tolerance) {
        throw std::domain_error("Risk-neutral probability outside [0,1]: increase steps");
    }
    const double up_probability = std::clamp(p, 0.0, 1.0);
    const double down_probability = 1.0 - up_probability;
    const double discount = detail::safe_exp(-c.annual_rate * dt);

    std::vector<double> value(c.steps + 1);
    for (std::size_t j = 0; j <= c.steps; ++j) {
        const double exponent = (2.0 * static_cast<double>(j) - n) * h;
        const double stock = detail::checked(c.spot * detail::safe_exp(exponent));
        value[j] = detail::intrinsic(stock, c.strike, c.kind);
    }

    for (std::size_t i = c.steps; i > 0; --i) {
        for (std::size_t j = 0; j < i; ++j) {
            const double hold = detail::checked(discount *
                (down_probability * value[j] + up_probability * value[j + 1]));
            if (c.exercise == ExerciseStyle::american) {
                const double exponent =
                    (2.0 * static_cast<double>(j) - static_cast<double>(i - 1)) * h;
                const double stock = detail::checked(c.spot * detail::safe_exp(exponent));
                value[j] = std::max(hold, detail::intrinsic(stock, c.strike, c.kind));
            } else {
                value[j] = hold;
            }
        }
    }
    return value[0];
}

// Independent analytical benchmark for European options only.
[[nodiscard]] inline double black_scholes_european(const Contract& c) {
    detail::validate(c);
    if (c.exercise != ExerciseStyle::european) {
        throw std::invalid_argument("Black-Scholes benchmark requires European exercise");
    }
    if (c.annual_volatility == 0.0) {
        return detail::deterministic_price(c);
    }
    const double sigma_t = c.annual_volatility * std::sqrt(c.maturity_years);
    const double d1 =
        (std::log(c.spot / c.strike) +
         (c.annual_rate - c.annual_dividend_yield +
          0.5 * c.annual_volatility * c.annual_volatility) * c.maturity_years) /
        sigma_t;
    const double d2 = d1 - sigma_t;
    constexpr double inv_sqrt_two = 0.7071067811865475244;
    const auto phi = [](double x) noexcept {
        return 0.5 * std::erfc(-x * inv_sqrt_two);
    };
    const double disc_spot = detail::checked(
        c.spot * detail::safe_exp(-c.annual_dividend_yield * c.maturity_years));
    const double disc_strike = detail::checked(
        c.strike * detail::safe_exp(-c.annual_rate * c.maturity_years));
    if (c.kind == OptionKind::call) {
        return detail::checked(disc_spot * phi(d1) - disc_strike * phi(d2));
    }
    return detail::checked(disc_strike * phi(-d2) - disc_spot * phi(-d1));
}

} // namespace quant::binomial

#pragma once

#include <cmath>
#include <stdexcept>

namespace quant14 {

struct EuropeanOptionPair {
    double stock_price;
    double strike;
    double annual_rate;
    double years_to_expiry;
    double call_price;
    double put_price;
};

struct ParityCheck {
    double quoted_difference;
    double theoretical_difference;
    double absolute_error;
    bool within_tolerance;
};

// European options, same expiry/strike, no dividends, continuously compounded rate.
[[nodiscard]] inline ParityCheck check_parity(
    const EuropeanOptionPair& q, double tolerance = 1e-9) {
    const auto finite = [](double x) { return std::isfinite(x); };
    if (!finite(q.stock_price) || !finite(q.strike) || !finite(q.annual_rate) ||
        !finite(q.years_to_expiry) || !finite(q.call_price) ||
        !finite(q.put_price) || !finite(tolerance) ||
        q.stock_price <= 0.0 || q.strike <= 0.0 ||
        q.years_to_expiry < 0.0 || q.call_price < 0.0 ||
        q.put_price < 0.0 || tolerance < 0.0) {
        throw std::invalid_argument("Invalid European option pair or tolerance");
    }

    const double discounted_strike =
        q.strike * std::exp(-q.annual_rate * q.years_to_expiry);
    if (!finite(discounted_strike)) {
        throw std::overflow_error("Discounted strike is not finite");
    }
    const double theoretical = q.stock_price - discounted_strike;
    const double quoted = q.call_price - q.put_price;
    const double error = std::abs(quoted - theoretical);
    if (!finite(error)) {
        throw std::overflow_error("Parity difference is not finite");
    }
    return {quoted, theoretical, error, error <= tolerance};
}

}  // namespace quant14

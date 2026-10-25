#pragma once

#include <cmath>
#include <stdexcept>

namespace quant15 {

struct AnnualBond {
    double face_value;
    double annual_coupon_rate;
    int years_to_maturity;
};

inline void validate(const AnnualBond& b) {
    if (!std::isfinite(b.face_value) || !std::isfinite(b.annual_coupon_rate) ||
        b.face_value <= 0.0 || b.annual_coupon_rate < 0.0 ||
        b.years_to_maturity < 1 || b.years_to_maturity > 100) {
        throw std::invalid_argument("Expected finite face value, nonnegative coupon, 1-100 years");
    }
}

[[nodiscard]] inline double bond_price(const AnnualBond& b, double annual_yield) {
    validate(b);
    if (!std::isfinite(annual_yield) || annual_yield < 0.0) {
        throw std::invalid_argument("Yield must be finite and nonnegative");
    }
    const double factor = 1.0 + annual_yield;
    const double coupon = b.face_value * b.annual_coupon_rate;
    double total = 0.0;
    double discount = 1.0;
    for (int year = 1; year <= b.years_to_maturity; ++year) {
        discount /= factor;
        total += coupon * discount;
    }
    total += b.face_value * discount;
    if (!std::isfinite(total)) throw std::overflow_error("Price overflow");
    return total;
}

// Bisection exploits the strictly decreasing price/yield relationship.
[[nodiscard]] inline double yield_to_maturity(
    const AnnualBond& b, double market_price, double absolute_tolerance = 1e-9) {
    validate(b);
    if (!std::isfinite(market_price) || market_price <= 0.0 ||
        !std::isfinite(absolute_tolerance) || absolute_tolerance <= 0.0) {
        throw std::invalid_argument("Market price and tolerance must be positive and finite");
    }

    const double price_at_zero = bond_price(b, 0.0);
    if (market_price > price_at_zero + absolute_tolerance) {
        throw std::domain_error("Quote implies a negative yield, outside the supported domain");
    }
    if (std::abs(market_price - price_at_zero) <= absolute_tolerance) return 0.0;

    double low = 0.0;
    double high = 0.1;
    while (bond_price(b, high) > market_price) {
        high *= 2.0;
        if (high > 1e6) {
            throw std::domain_error("Yield outside search range");
        }
    }
    for (int iteration = 0; iteration < 200; ++iteration) {
        const double mid = low + (high - low) / 2.0;
        const double calculated = bond_price(b, mid);
        if (std::abs(calculated - market_price) <= absolute_tolerance) return mid;
        if (calculated > market_price) low = mid;
        else high = mid;
    }
    return low + (high - low) / 2.0;
}

}  // namespace quant15

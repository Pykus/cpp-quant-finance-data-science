#include "pricing.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace q = quant::antithetic;
static std::size_t checks = 0;
static std::size_t failures = 0;

void check(bool ok, std::string_view description) {
    ++checks;
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << description << '\n';
    }
}
void near(double actual, double expected, double tolerance, std::string_view label) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, label);
}
template <class F>
void invalid(F&& f, std::string_view description) {
    try {
        f();
        check(false, description);
    } catch (const std::invalid_argument&) {
        check(true, description);
    } catch (...) {
        check(false, description);
    }
}

int main() {
    const q::EuropeanCall option{100.0, 100.0, 0.05, 0.20, 1.0};
    const double bs = q::black_scholes(option);
    near(bs, 10.450583572185565, 1e-10, "analytic reference");

    constexpr std::size_t pairs = 150'000;
    const auto plain = q::estimate(option, pairs, 2026, q::Method::independent);
    const auto paired = q::estimate(option, pairs, 2026, q::Method::antithetic);

    check(plain.payoffs == 2 * pairs, "plain payoff count");
    check(paired.payoffs == 2 * pairs, "paired payoff count");
    check(plain.observations == 2 * pairs, "plain independent units");
    check(paired.observations == pairs, "paired independent units");
    check(plain.price > 0.0 && paired.price > 0.0, "positive option prices");
    check(plain.variance >= 0.0 && paired.variance >= 0.0, "nonnegative variances");
    check(plain.standard_error > 0.0 && paired.standard_error > 0.0, "positive SE");
    check(paired.standard_error < plain.standard_error, "variance reduction in fixture");
    check(std::abs(plain.price - bs) < 5 * plain.standard_error, "plain near benchmark");
    check(std::abs(paired.price - bs) < 5 * paired.standard_error, "paired near benchmark");
    near(paired.price, q::estimate(option, pairs, 2026, q::Method::antithetic).price,
         0.0, "same seed reproducibility");

    for (const auto e : {plain, paired}) {
        check(e.low95 <= e.price && e.price <= e.high95, "interval contains its mean");
        near(e.high95 - e.price, e.price - e.low95, 1e-12, "symmetric interval");
        check(std::isfinite(e.price) && std::isfinite(e.standard_error),
              "finite estimate");
    }
    const q::EuropeanCall other{90.0, 130.0, -0.01, 0.15, 2.0};
    const auto low = q::estimate(other, 10'000, 7, q::Method::antithetic);
    check(std::isfinite(low.price) && low.price >= 0.0, "OTM and negative rate");

    invalid([&] { (void)q::estimate(option, 0, 0, q::Method::independent); },
            "zero pairs rejected");
    invalid([&] { (void)q::estimate(option, 1, 0, q::Method::antithetic); },
            "one pair rejected");
    invalid([&] {
        (void)q::estimate(option, std::numeric_limits<std::size_t>::max(),
                          0, q::Method::independent);
    }, "count overflow rejected");
    invalid([&] { (void)q::estimate(option, 2, 0, static_cast<q::Method>(99)); },
            "unknown method rejected");

    constexpr std::array bad{
        q::EuropeanCall{0, 100, 0.05, 0.20, 1},
        q::EuropeanCall{-1, 100, 0.05, 0.20, 1},
        q::EuropeanCall{100, 0, 0.05, 0.20, 1},
        q::EuropeanCall{100, 100, 0.05, 0, 1},
        q::EuropeanCall{100, 100, 0.05, -0.2, 1},
        q::EuropeanCall{100, 100, 0.05, 0.2, 0},
        q::EuropeanCall{100, 100, std::numeric_limits<double>::infinity(), 0.2, 1},
        q::EuropeanCall{100, 100, 0.05, std::numeric_limits<double>::quiet_NaN(), 1},
        q::EuropeanCall{100, 100, 0.05, 0.2, std::numeric_limits<double>::infinity()}
    };
    for (const auto& x : bad) {
        invalid([&] { (void)q::estimate(x, 2, 123, q::Method::antithetic); },
                "invalid Monte Carlo input");
        invalid([&] { (void)q::black_scholes(x); }, "invalid analytic input");
    }

    std::cout << (failures == 0 ? "PASS: " : "FAIL: ") << checks
              << " assertions, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}

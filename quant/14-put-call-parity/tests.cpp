#include "parity.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
int checks = 0;
void expect(bool condition, const char* message) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
template <typename Exception, typename F>
void expect_throws(F&& f) {
    bool caught = false;
    try { f(); } catch (const Exception&) { caught = true; }
    expect(caught, "expected exception");
}
}

int main() {
    using quant14::EuropeanOptionPair;
    using quant14::check_parity;
    const double s = 100.0;
    const double k = 100.0;
    const double r = 0.05;
    const double t = 1.0;
    const double put = 10.0 - s + k * std::exp(-r * t);
    const auto base = check_parity({s, k, r, t, 10.0, put});
    expect(base.within_tolerance, "balanced quotes satisfy parity");
    expect(base.absolute_error < 1e-12, "low rounding error");
    expect(std::abs(base.theoretical_difference - 4.8770575499286) < 1e-10,
           "worked example");

    auto bad = check_parity({s, k, r, t, 10.0, put + 0.01}, 1e-6);
    expect(!bad.within_tolerance, "mispriced option pair detected");
    expect(std::abs(bad.absolute_error - 0.01) < 1e-10, "error magnitude");
    expect(check_parity({100, 100, 0, 0, 1, 1}, 0).within_tolerance,
           "zero maturity, zero interest");

    expect_throws<std::invalid_argument>([] {
        (void)check_parity({-1, 100, 0, 1, 2, 3});
    });
    expect_throws<std::invalid_argument>([] {
        (void)check_parity({100, 100, 0, 1, -1, 3});
    });
    expect_throws<std::invalid_argument>([] {
        (void)check_parity({100, 100, 0, 1, 2, 3}, -1);
    });
    expect_throws<std::invalid_argument>([] {
        (void)check_parity({100, 100, 0, std::numeric_limits<double>::quiet_NaN(), 2, 3});
    });
    expect_throws<std::overflow_error>([] {
        (void)check_parity({100, 100, -1000, 100, 2, 3});
    });

    // Deterministic stress cases with consistent nonnegative synthetic quotes.
    for (int i = 0; i != 10000; ++i) {
        const double stock = 80.0 + (i % 100);
        const double strike = 75.0 + (i % 113);
        const double rate = (i % 11 - 5) * 0.01;
        const double years = 0.05 + (i % 15) * 0.08;
        const double discount = strike * std::exp(-rate * years);
        const double call = std::max(0.0, stock - discount) + 2.0;
        const double put_price = call - stock + discount;
        const auto check = check_parity({stock, strike, rate, years, call, put_price}, 1e-10);
        expect(check.within_tolerance, "consistent synthetic price");
    }

    std::cout << "PASS " << checks << " checks\n";
}

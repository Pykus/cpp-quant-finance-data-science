#include "greeks.hpp"

#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

int assertions = 0;

void check(bool valid, std::string_view why) {
    ++assertions;
    if (!valid) {
        throw std::runtime_error(std::string(why));
    }
}

void near(double actual, double expected, double tolerance,
          std::string_view why) {
    check(std::isfinite(actual) &&
          std::abs(actual - expected) <= tolerance, why);
}

void rejects_invalid(const std::function<void()>& fn,
                     std::string_view why) {
    bool rejected = false;
    try {
        fn();
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    check(rejected, why);
}

void textbook_case() {
    const quant::OptionInputs x{100.0, 100.0, 0.05, 0.0, 0.2, 1.0};
    const auto call = quant::black_scholes(quant::OptionKind::call, x);
    const auto put = quant::black_scholes(quant::OptionKind::put, x);
    near(call.price, 10.4505835721856, 1e-9, "textbook call");
    near(put.price, 5.57352602225697, 1e-9, "textbook put");
    near(call.delta, 0.636830651175619, 1e-12, "call delta");
    near(put.delta, -0.363169348824381, 1e-12, "put delta");
    near(call.gamma, 0.0187620173458469, 1e-12, "gamma");
    near(call.vega, 37.5240346916938, 1e-10, "vega");
    near(call.gamma, put.gamma, 1e-15, "shared gamma");
    near(call.vega, put.vega, 1e-12, "shared vega");

    const double shares = quant::shares_for_zero_delta(
        -1.0, 100.0, call.delta);
    near(shares, 63.6830651175619, 1e-10, "short call hedge");
    near(-100.0 * call.delta + shares, 0.0, 1e-12, "zero delta");
    near(quant::shares_for_zero_delta(
        2.0, 100.0, put.delta), 72.6338697648762, 1e-10,
        "two long puts hedge");
}

void cross_check_grid() {
    // Include negative rates, dividends, out-of-the-money options,
    // different volatilities and short / long maturities.
    for (double rate : {-0.01, 0.04}) {
        for (double dividend : {0.0, 0.03}) {
            for (double strike : {80.0, 100.0, 125.0}) {
                for (double volatility : {0.13, 0.35}) {
                    for (double years : {0.25, 1.5}) {
                        const quant::OptionInputs x{
                            100.0, strike, rate, dividend,
                            volatility, years};
                        const auto call = quant::black_scholes(
                            quant::OptionKind::call, x);
                        const auto put = quant::black_scholes(
                            quant::OptionKind::put, x);
                        const double discounted_spot =
                            x.spot * std::exp(-dividend * years);
                        const double discounted_strike =
                            strike * std::exp(-rate * years);
                        near(call.price - put.price,
                             discounted_spot - discounted_strike,
                             1e-10, "put-call parity");
                        near(call.delta - put.delta,
                             std::exp(-dividend * years),
                             1e-12, "delta parity");
                        near(call.gamma, put.gamma, 1e-14,
                             "call-put gamma");
                        near(call.vega, put.vega, 1e-12,
                             "call-put vega");
                        check(call.price >= -1e-12, "positive call price");
                        check(put.price >= -1e-12, "positive put price");
                        check(call.delta >= -1e-12, "positive call delta");
                        check(put.delta <= 1e-12, "negative put delta");

                        for (auto kind : {quant::OptionKind::call,
                                          quant::OptionKind::put}) {
                            const auto g = quant::black_scholes(kind, x);
                            near(g.delta,
                                 quant::finite_difference_delta(kind, x),
                                 5e-5, "finite difference delta");
                            near(g.gamma,
                                 quant::finite_difference_gamma(kind, x),
                                 8e-5, "finite difference gamma");
                            near(g.vega,
                                 quant::finite_difference_vega(kind, x),
                                 1e-3, "finite difference vega");
                        }
                    }
                }
            }
        }
    }
}

void inputs_and_extremes() {
    const quant::OptionInputs valid{100.0, 100.0, 0.03, 0.02, 0.2, 1.0};
    auto x = valid;
    x.spot = 0.0;
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::call, x); }, "zero spot");
    x = valid; x.strike = -1.0;
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::call, x); }, "negative strike");
    x = valid; x.volatility = 0.0;
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::call, x); }, "zero volatility");
    x = valid; x.maturity_years = 0.0;
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::call, x); }, "expired option");
    x = valid; x.rate = std::numeric_limits<double>::quiet_NaN();
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::call, x); }, "NaN rate");
    x = valid; x.dividend_yield =
        std::numeric_limits<double>::infinity();
    rejects_invalid([&] { (void)quant::black_scholes(
        quant::OptionKind::put, x); }, "infinite dividend");

    rejects_invalid([&] { (void)quant::finite_difference_delta(
        quant::OptionKind::call, valid, 0.0); }, "zero delta bump");
    rejects_invalid([&] { (void)quant::finite_difference_gamma(
        quant::OptionKind::call, valid, 1.0); }, "oversized gamma bump");
    rejects_invalid([&] { (void)quant::finite_difference_vega(
        quant::OptionKind::call, valid, 0.2); }, "oversized vega bump");
    rejects_invalid([&] { (void)quant::shares_for_zero_delta(
        1.0, 0.0, 0.5); }, "zero multiplier");
    rejects_invalid([&] { (void)quant::shares_for_zero_delta(
        std::numeric_limits<double>::quiet_NaN(), 100.0, 0.5);
        }, "NaN contracts");

    const auto x_small = quant::OptionInputs{
        100.0, 100.0, -0.02, 0.0, 0.5, 0.01};
    const auto small = quant::black_scholes(
        quant::OptionKind::put, x_small);
    check(std::isfinite(small.price), "short-term put finite");
    check(small.gamma > 0.0, "short-term gamma positive");
    check(small.vega > 0.0, "short-term vega positive");
}

} // namespace

int main() {
    try {
        textbook_case();
        cross_check_grid();
        inputs_and_extremes();
        std::cout << assertions << " assertions passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAILED after " << assertions << " assertions: "
                  << ex.what() << '\n';
        return 1;
    }
}

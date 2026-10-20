#include "implied_volatility.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

std::size_t checks = 0;

void check(bool condition, std::string_view description) {
    ++checks;
    if (!condition) {
        throw std::runtime_error(std::string(description));
    }
}

void near(double actual, double expected, double tolerance, std::string_view description) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, description);
}

template <typename Exception, typename Fn>
void throws(Fn&& action, std::string_view description) {
    bool caught = false;
    try {
        std::invoke(std::forward<Fn>(action));
    } catch (const Exception&) {
        caught = true;
    }
    check(caught, description);
}

}  // namespace

int main() {
    using quant::OptionInputs;
    using quant::OptionKind;
    using quant::black_scholes_price;
    using quant::black_scholes_vega;
    using quant::implied_volatility;
    using quant::arbitrage_bounds;

    try {
        const OptionInputs standard{100.0, 100.0, 0.05, 0.0, 1.0};
        const double call = black_scholes_price(OptionKind::call, standard, 0.2);
        const double put = black_scholes_price(OptionKind::put, standard, 0.2);

        near(call, 10.450583572185565, 1e-11, "textbook European call");
        near(put, 5.573526022256971, 1e-11, "textbook European put");
        near(call - put, 100.0 - 100.0 * std::exp(-0.05), 1e-11,
             "put-call parity");
        near(black_scholes_vega(standard, 0.2), 37.52403469169379, 1e-10,
             "analytic vega reference");
        check(black_scholes_vega(standard, 0.2) > 0.0, "positive vega");

        const auto call_bounds = arbitrage_bounds(OptionKind::call, standard);
        const auto put_bounds = arbitrage_bounds(OptionKind::put, standard);
        near(call_bounds.lower, 100.0 - 100.0 * std::exp(-0.05), 1e-12,
             "call no-arbitrage lower");
        near(call_bounds.upper, 100.0, 1e-12, "call upper");
        near(put_bounds.lower, 0.0, 1e-12, "put lower");
        near(put_bounds.upper, 100.0 * std::exp(-0.05), 1e-12, "put upper");

        near(black_scholes_price(OptionKind::call, standard, 0.0),
             call_bounds.lower, 1e-12, "zero volatility call limit");
        near(black_scholes_price(OptionKind::put, standard, 0.0),
             put_bounds.lower, 1e-12, "zero volatility put limit");
        near(implied_volatility(OptionKind::call, standard, call_bounds.lower).volatility,
             0.0, 0.0, "zero volatility inverse");
        check(implied_volatility(OptionKind::call, standard, call_bounds.lower).iterations == 0,
              "zero volatility has no iterations");

        const auto result = implied_volatility(OptionKind::call, standard, call);
        near(result.volatility, 0.2, 1e-9, "recover textbook call volatility");
        check(result.iterations > 0 && result.iterations <= 200,
              "iteration count is bounded");
        check(result.absolute_repricing_error <= 1e-10,
              "repricing error contract");
        near(implied_volatility(OptionKind::put, standard, put).volatility,
             0.2, 1e-9, "recover textbook put volatility");

        const std::array<OptionInputs, 4> markets{{
            {80.0, 100.0, 0.01, 0.02, 1.0},
            {120.0, 100.0, 0.03, 0.01, 0.5},
            {100.0, 110.0, -0.01, 0.0, 2.0},
            {100.0, 90.0, 0.0, 0.05, 3.0}
        }};
        const std::array<double, 4> volatilities{{0.15, 0.35, 0.65, 1.75}};
        for (const auto& inputs : markets) {
            for (const double volatility : volatilities) {
                const double call_quote = black_scholes_price(
                    OptionKind::call, inputs, volatility);
                const double put_quote = black_scholes_price(
                    OptionKind::put, inputs, volatility);
                near(implied_volatility(OptionKind::call, inputs, call_quote).volatility,
                     volatility, 1e-7, "round-trip call across market conditions");
                near(implied_volatility(OptionKind::put, inputs, put_quote).volatility,
                     volatility, 1e-7, "round-trip put across market conditions");
                const double asset_pv = inputs.spot * std::exp(-inputs.dividend_yield
                    * inputs.years_to_expiry);
                const double strike_pv = inputs.strike * std::exp(-inputs.rate
                    * inputs.years_to_expiry);
                near(call_quote - put_quote, asset_pv - strike_pv, 1e-9,
                     "put-call parity with dividends and negative rates");
            }
        }

        const OptionInputs forward_atm{100.0, 100.0, 0.0, 0.0, 0.01};
        const double short_quote = black_scholes_price(
            OptionKind::call, forward_atm, 0.3);
        near(implied_volatility(OptionKind::call, forward_atm, short_quote).volatility,
             0.3, 1e-8, "short maturity round trip");

        check(black_scholes_price(OptionKind::call, standard, 0.5) > call,
              "call price increases with volatility");
        check(black_scholes_price(OptionKind::put, standard, 0.5) > put,
              "put price increases with volatility");
        const double epsilon = 1e-5;
        const double central_vega = (
            black_scholes_price(OptionKind::call, standard, 0.2 + epsilon) -
            black_scholes_price(OptionKind::call, standard, 0.2 - epsilon))
            / (2.0 * epsilon);
        near(central_vega, black_scholes_vega(standard, 0.2), 1e-7,
             "finite difference verifies analytic vega");

        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call, {0.0, 100.0, 0.0, 0.0, 1.0}); },
            "reject zero spot");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call, {100.0, -1.0, 0.0, 0.0, 1.0}); },
            "reject negative strike");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call, {100.0, 100.0, 0.0, 0.0, 0.0}); },
            "reject expired contract");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call,
                  {100.0, 100.0, std::numeric_limits<double>::infinity(), 0.0, 1.0}); },
            "reject nonfinite rate");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::put, {100.0, 100.0, 0.0, 0.0, -1.0}); },
            "reject negative time");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call, {100.0, 100.0, 0.0, -1e3, 1.0}); },
            "reject discount overflow");
        throws<std::invalid_argument>(
            [&] { (void)arbitrage_bounds(OptionKind::call, {100.0, 100.0, 1e3, 0.0, 1.0}); },
            "reject discount underflow");
        throws<std::invalid_argument>(
            [&] { (void)black_scholes_price(OptionKind::call, standard, -0.1); },
            "reject negative volatility");
        throws<std::invalid_argument>(
            [&] { (void)black_scholes_price(OptionKind::call, standard,
                  std::numeric_limits<double>::quiet_NaN()); },
            "reject NaN volatility");
        throws<std::invalid_argument>(
            [&] { (void)black_scholes_vega(standard, 0.0); },
            "reject zero volatility vega");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard, -1.0); },
            "reject negative quote");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard, 0.0); },
            "reject below intrinsic");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard, call_bounds.upper); },
            "reject upper bound");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::put, standard, put_bounds.upper); },
            "reject put upper bound");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard,
                  std::numeric_limits<double>::quiet_NaN()); },
            "reject nonfinite market quote");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard, call, 0.0); },
            "reject zero tolerance");
        throws<std::invalid_argument>(
            [&] { (void)implied_volatility(OptionKind::call, standard, call, 1e-10, 0); },
            "reject no iterations");
        throws<std::runtime_error>(
            [&] { (void)implied_volatility(OptionKind::call, standard,
                  black_scholes_price(OptionKind::call, standard, 0.234), 1e-15, 1); },
            "report non-convergence");

        std::cout << "PASS " << checks << "/" << checks << " independent checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}

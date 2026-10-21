#include "binomial.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

using quant::binomial::Contract;
using quant::binomial::ExerciseStyle;
using quant::binomial::OptionKind;
using quant::binomial::black_scholes_european;
using quant::binomial::price;

namespace {
std::size_t checks = 0;
std::size_t failures = 0;

void check(bool ok, std::string_view label) {
    ++checks;
    if (!ok) {
        ++failures;
        std::cerr << "FAIL " << label << '\n';
    }
}
void near(double actual, double expected, double tol, std::string_view label) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tol, label);
}
template<class E, class F>
void rejects(F&& f, std::string_view label) {
    try { f(); check(false, label); }
    catch (const E&) { check(true, label); }
    catch (...) { check(false, label); }
}

Contract sample() {
    return {100.0, 100.0, 1.0, 0.05, 0.0, 0.20,
            400, OptionKind::call, ExerciseStyle::european};
}
void prices_and_parity() {
    auto c = sample();
    near(black_scholes_european(c), 10.4505835722, 1e-9, "BS call reference");
    near(price(c), 10.4505835722, .006, "CRR call convergence");
    c.kind = OptionKind::put;
    near(black_scholes_european(c), 5.5735260223, 1e-9, "BS put reference");
    near(price(c), 5.5735260223, .006, "CRR put convergence");

    c = sample();
    c.steps = 1;
    c.annual_rate = 0.0;
    c.annual_volatility = std::log(1.1);
    const double p = (1.0 - 1.0 / 1.1) / (1.1 - 1.0 / 1.1);
    near(price(c), p * 10.0, 1e-12, "manual one-step call");
    c.kind = OptionKind::put;
    near(price(c), (1.0 - p) * (100.0 - 100.0 / 1.1), 1e-12,
         "manual one-step put");

    for (double q : {0.0, .01, .04}) {
        for (std::size_t steps : {1U, 2U, 15U, 200U}) {
            c = sample();
            c.steps = steps;
            c.annual_dividend_yield = q;
            const double call = price(c);
            c.kind = OptionKind::put;
            const double put = price(c);
            const double rhs = c.spot * std::exp(-q * c.maturity_years) -
                               c.strike * std::exp(-c.annual_rate * c.maturity_years);
            near(call - put, rhs, 1e-10, "European put-call parity");
        }
    }
}
void exercise_choices() {
    for (OptionKind kind : {OptionKind::call, OptionKind::put}) {
        for (std::size_t steps : {1U, 2U, 20U, 250U}) {
            auto c = sample();
            c.kind = kind;
            c.steps = steps;
            c.spot = 90.0;
            const double european = price(c);
            c.exercise = ExerciseStyle::american;
            const double american = price(c);
            check(american + 1e-12 >= european, "American >= European");
            check(american + 1e-12 >=
                  quant::binomial::detail::intrinsic(c.spot, c.strike, kind),
                  "American >= immediate intrinsic");
        }
    }
    auto c = sample();
    c.spot = 80.0;
    c.strike = 100.0;
    c.annual_rate = .08;
    c.kind = OptionKind::put;
    const double european_put = price(c);
    c.exercise = ExerciseStyle::american;
    check(price(c) > european_put + 1.0, "American put has exercise premium");

    for (std::size_t steps : {1U, 2U, 10U, 400U}) {
        c = sample();
        c.steps = steps;
        const double european_call = price(c);
        c.exercise = ExerciseStyle::american;
        near(price(c), european_call, 1e-10, "No-dividend American call");
    }
    c = sample();
    c.spot = 130.0;
    c.annual_rate = .01;
    c.annual_dividend_yield = .20;
    c.exercise = ExerciseStyle::american;
    const double american_call = price(c);
    c.exercise = ExerciseStyle::european;
    check(american_call > price(c) + .01, "Dividends can favor early call exercise");
}
void deterministic_and_convergence() {
    auto c = sample();
    c.annual_volatility = 0.0;
    near(price(c), 100.0 - 100.0 * std::exp(-.05), 1e-10,
         "Zero-volatility European call");
    near(black_scholes_european(c), price(c), 1e-12,
         "BS zero-volatility limit");
    c.exercise = ExerciseStyle::american;
    near(price(c), 100.0 - 100.0 * std::exp(-.05), 1e-10,
         "Zero-volatility American call");
    c.spot = 80.0;
    c.strike = 100.0;
    c.annual_rate = .08;
    c.kind = OptionKind::put;
    near(price(c), 20.0, 1e-12, "Deterministic American put immediate exercise");
    c.exercise = ExerciseStyle::european;
    near(price(c), 100.0 * std::exp(-.08) - 80.0, 1e-10,
         "Deterministic European put");
    c.spot = c.strike = 100.0;
    c.annual_rate = c.annual_dividend_yield = 0.0;
    near(price(c), 0.0, 1e-12, "Zero carry at-money European put");
    c.exercise = ExerciseStyle::american;
    near(price(c), 0.0, 1e-12, "Zero carry at-money American put");
    c.kind = OptionKind::call;
    c.annual_dividend_yield = .15;
    near(price(c), 0.0, 1e-12, "Deterministic dividend-paying call");
    c.strike = 60.0;
    near(price(c), 40.0, 1e-12, "American call exercises immediately");

    c = sample();
    const double bs = black_scholes_european(c);
    double prev = std::numeric_limits<double>::infinity();
    for (std::size_t steps : {50U, 100U, 200U, 400U, 800U}) {
        c.steps = steps;
        const double err = std::abs(price(c) - bs);
        check(err < prev, "Even-step CRR convergence");
        prev = err;
    }
}
void sensitivity_and_errors() {
    auto c = sample();
    c.spot = 80.0;
    const double low_spot = price(c);
    c.spot = 120.0;
    check(price(c) > low_spot, "Call grows with spot");
    c.spot = 100.0;
    c.strike = 90.0;
    const double low_strike_call = price(c);
    c.strike = 110.0;
    check(price(c) < low_strike_call, "Call falls with strike");
    c.kind = OptionKind::put;
    const double high_strike_put = price(c);
    c.strike = 90.0;
    check(price(c) < high_strike_put, "Put grows with strike");
    c.annual_rate = -.01;
    check(std::isfinite(price(c)), "Negative interest rate supported");
    c = sample();
    c.annual_rate = 0.0;
    c.annual_volatility = 1e-8;
    near(price(c), black_scholes_european(c), 1e-9,
         "Numerical stability at tiny positive volatility");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    for (double value : {0.0, -1.0, nan, inf}) {
        c = sample(); c.spot = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid spot");
        c = sample(); c.strike = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid strike");
        c = sample(); c.maturity_years = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid maturity");
    }
    for (double value : {-0.1, nan, inf}) {
        c = sample(); c.annual_volatility = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid volatility");
    }
    for (double value : {nan, inf, -inf}) {
        c = sample(); c.annual_rate = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid rate");
        c = sample(); c.annual_dividend_yield = value;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid yield");
    }
    for (std::size_t steps : {0U, 5001U}) {
        c = sample(); c.steps = steps;
        rejects<std::invalid_argument>([&] { (void)price(c); }, "Invalid step count");
    }
    c = sample(); c.steps = 1; c.annual_rate = .5; c.annual_volatility = .01;
    rejects<std::domain_error>([&] { (void)price(c); }, "Invalid up probability");
    c.annual_rate = 0.0; c.annual_dividend_yield = .5;
    rejects<std::domain_error>([&] { (void)price(c); }, "Invalid down probability");
    c = sample(); c.exercise = ExerciseStyle::american;
    rejects<std::invalid_argument>([&] { (void)black_scholes_european(c); },
                                   "BS rejects American exercise");
    c = sample(); c.spot = 1e308; c.annual_volatility = 1.0;
    c.maturity_years = 5.0; c.steps = 5;
    rejects<std::overflow_error>([&] { (void)price(c); }, "Overflow detected");
}
} // namespace

int main() {
    prices_and_parity();
    exercise_choices();
    deterministic_and_convergence();
    sensitivity_and_errors();
    std::cout << "Checks: " << checks << ", failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}

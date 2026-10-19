#include "ewma.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;

void check(bool condition, const char* message) {
    ++checks;
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void near(double actual, double expected, double tolerance, const char* label) {
    check(std::abs(actual - expected) <= tolerance, label);
}

template <typename Error, typename Func>
void throws(Func operation, const char* message) {
    bool caught = false;
    try {
        operation();
    } catch (const Error&) {
        caught = true;
    }
    check(caught, message);
}

void run() {
    const std::vector<double> sample{0.01, -0.02, 0.03, -0.01, 0.0};
    const auto out = quant::ewma_risk(sample, 0.94, 100'000.0);
    check(out.size() == 5, "five snapshots");
    for (std::size_t i = 0; i < out.size(); ++i) {
        check(out[i].day == i + 1, "day indexed from 1");
        check(out[i].variance >= 0.0, "nonnegative variance");
        check(out[i].daily_volatility >= 0.0, "nonnegative volatility");
        check(out[i].next_day_var95 >= 0.0, "nonnegative VaR");
        near(out[i].daily_volatility * out[i].daily_volatility,
             out[i].variance, 1e-12, "volatility is sqrt(variance)");
    }
    near(out[0].variance, 0.0001, 1e-14, "first squared return seeds the model");
    near(out[1].variance, 0.000118, 1e-14, "first update");
    near(out[2].variance, 0.00016492, 1e-14, "second update");
    near(out[3].variance, 0.0001610248, 1e-14, "third update");
    near(out[4].variance, 0.000151363312, 1e-14, "zero return decays risk");
    near(out[0].daily_volatility, 0.01, 1e-14, "initial daily volatility");
    near(out[0].annualized_volatility, 0.01 * std::sqrt(252.0),
         1e-12, "sqrt time annualization");
    near(out[0].next_day_var95, quant::normal_95_quantile * 1000.0,
         1e-9, "parametric loss threshold");

    const auto equal_weight = quant::ewma_risk(sample, 0.0, 100.0);
    near(equal_weight[1].variance, 0.0004, 1e-14, "lambda zero tracks latest return");
    near(equal_weight[2].variance, 0.0009, 1e-14, "zero decay always latest squared");
    near(equal_weight[4].variance, 0.0, 1e-14, "last observation is zero");

    const std::vector<double> zeros{0.0, 0.0, 0.0};
    const auto zero_risk = quant::ewma_risk(zeros, 0.94, 1000.0);
    check(zero_risk.size() == zeros.size(), "all-zero series length");
    near(zero_risk.back().next_day_var95, 0.0, 1e-14, "zero risk when all returns zero");

    const auto no_capital = quant::ewma_risk(sample, 0.94, 0.0);
    near(no_capital[0].next_day_var95, 0.0, 1e-14, "zero portfolio value");
    near(no_capital[0].daily_volatility, 0.01, 1e-14, "capital does not affect volatility");

    const auto annual = quant::ewma_risk(sample, 0.94, 100.0, 365.0);
    near(annual[0].annualized_volatility, std::sqrt(365.0) * 0.01,
         1e-12, "custom annualization");

    throws<std::invalid_argument>([]{ (void)quant::ewma_risk({}, 0.94, 100.0); }, "empty returns");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, -0.1, 100.0); }, "negative lambda");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 1.0, 100.0); }, "lambda one");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 1.01, 100.0); }, "lambda too large");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, std::numeric_limits<double>::quiet_NaN(), 100.0); }, "nan lambda");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 0.9, -1.0); }, "negative value");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 0.9, std::numeric_limits<double>::infinity()); }, "infinite value");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 0.9, 100.0, 0.0); }, "zero trading days");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 0.9, 100.0, -1.0); }, "negative trading days");
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(sample, 0.9, 100.0, std::numeric_limits<double>::infinity()); }, "infinite annualization");
    const std::vector<double> invalid_return{0.01, std::numeric_limits<double>::quiet_NaN()};
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(invalid_return, 0.9, 100.0); }, "nan return");
    const std::vector<double> infinite_return{std::numeric_limits<double>::infinity()};
    throws<std::invalid_argument>([&]{ (void)quant::ewma_risk(infinite_return, 0.9, 100.0); }, "infinite return");
    const std::vector<double> huge_return{std::numeric_limits<double>::max()};
    throws<std::overflow_error>([&]{ (void)quant::ewma_risk(huge_return, 0.9, 100.0); }, "squared-return overflow");
    throws<std::overflow_error>([&]{ (void)quant::ewma_risk(std::vector<double>{0.8}, 0.9, std::numeric_limits<double>::max()); }, "loss estimate overflow");
    check(sample[0] == 0.01, "input not modified");
}
} // namespace

int main() {
    try {
        run();
        std::cout << checks << " checks passed\n";
    } catch (const std::exception& ex) {
        std::cerr << "FAIL after " << checks << " checks: " << ex.what() << '\n';
        return 1;
    }
}

#include "drawdown.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
int passed = 0;

void check(bool condition, const char* description) {
    if (!condition) {
        throw std::runtime_error(description);
    }
    ++passed;
}

void approx(double actual, double expected, const char* description) {
    const double scale = std::max({1.0, std::abs(actual), std::abs(expected)});
    check(std::abs(actual - expected) <= 1e-12 * scale, description);
}

template <typename Exception, typename Function>
void throws_as(Function operation, const char* description) {
    bool caught = false;
    try {
        operation();
    } catch (const Exception&) {
        caught = true;
    }
    check(caught, description);
}
} // namespace

int main() {
    try {
        const auto sample = quant::analyze_drawdowns(
            {0.20, -0.25, 0.50, -0.20, 0.10}, 100.0);
        check(sample.path.size() == 6, "initial row plus five periods");
        const std::vector<double> expected_wealth{100.0, 120.0, 90.0, 135.0, 108.0, 118.8};
        const std::vector<double> expected_peaks{100.0, 120.0, 120.0, 135.0, 135.0, 135.0};
        const std::vector<double> expected_drawdowns{0.0, 0.0, 0.25, 0.0, 0.20, 0.12};
        for (std::size_t i = 0; i < sample.path.size(); ++i) {
            check(sample.path[i].period == i, "period index");
            approx(sample.path[i].wealth, expected_wealth[i], "wealth path");
            approx(sample.path[i].running_peak, expected_peaks[i], "high-water mark");
            approx(sample.path[i].drawdown, expected_drawdowns[i], "period drawdown");
        }
        approx(sample.cumulative_return, 0.188, "cumulative, not sum of returns");
        approx(sample.maximum_drawdown, 0.25, "maximum drawdown is 25 percent");
        check(sample.peak_period == 1, "peak before worst fall");
        check(sample.trough_period == 2, "trough at worst fall");

        const auto all_up = quant::analyze_drawdowns({0.10, 0.05}, 100.0);
        approx(all_up.path.back().wealth, 115.5, "compound two gains");
        approx(all_up.maximum_drawdown, 0.0, "no decline for rising series");
        check(all_up.peak_period == 0, "no maximum drawdown: peak index zero");
        check(all_up.trough_period == 0, "no maximum drawdown: trough index zero");

        const auto flat = quant::analyze_drawdowns({0.0, 0.0});
        approx(flat.cumulative_return, 0.0, "flat cumulative return");
        approx(flat.maximum_drawdown, 0.0, "flat maximum drawdown");

        const auto loss = quant::analyze_drawdowns({-1.0, 0.50, 0.10});
        approx(loss.path[1].wealth, 0.0, "100 percent loss gives zero");
        approx(loss.path.back().wealth, 0.0, "no recovery after total loss");
        approx(loss.maximum_drawdown, 1.0, "total loss gives full drawdown");
        check(loss.peak_period == 0 && loss.trough_period == 1, "total-loss timing");

        const auto tie = quant::analyze_drawdowns({0.20, -0.25, 1.0/3.0, -0.25});
        approx(tie.maximum_drawdown, 0.25, "same drawdown appears twice");
        check(tie.peak_period == 1 && tie.trough_period == 2, "first worst decline retained");

        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({}); }, "empty returns");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({0.1}, 0.0); }, "zero start");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({0.1}, -1.0); }, "negative start");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({-1.01}); }, "return below -100 percent");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({std::numeric_limits<double>::quiet_NaN()}); }, "NaN return");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({std::numeric_limits<double>::infinity()}); }, "infinite return");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({0.1}, std::numeric_limits<double>::infinity()); }, "infinite start");
        throws_as<std::invalid_argument>([] { (void)quant::analyze_drawdowns({0.1}, std::numeric_limits<double>::quiet_NaN()); }, "NaN start");
        throws_as<std::overflow_error>([] { (void)quant::analyze_drawdowns({1.0}, std::numeric_limits<double>::max()); }, "wealth overflow");

        std::cout << "PASS: " << passed << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << passed << " checks: " << error.what() << '\n';
        return 1;
    }
}

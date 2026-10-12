#include "discounting.hpp"

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

void check_near(double actual, double expected, double tolerance,
                const char* message) {
    check(std::isfinite(actual) && std::abs(actual - expected) <= tolerance,
          message);
}

template <typename Exception, typename Function>
void check_throws(Function action, const char* message) {
    bool caught = false;
    try {
        action();
    } catch (const Exception&) {
        caught = true;
    }
    check(caught, message);
}

}  // namespace

int main() {
    using quant::CashFlow;
    using quant::discount_factor;
    using quant::present_value;
    using quant::net_present_value;

    const std::vector<CashFlow> flows = {
        {0.0, -1000.0}, {1.0, 400.0}, {2.0, 400.0}, {3.0, 400.0}
    };
    const double reference = -1000.0 + 400.0 *
        (std::exp(-0.05) + std::exp(-0.10) + std::exp(-0.15));

    check_near(discount_factor(0.05, 1.0), std::exp(-0.05), 1e-14,
               "one-year discount factor");
    check_near(present_value({1.0, 105.0}, 0.05), 105.0 * std::exp(-0.05),
               1e-12, "single payment present value");
    check_near(net_present_value(flows, 0.05), reference, 1e-10,
               "three-year project reference NPV");
    check_near(net_present_value(flows, 0.0), 200.0, 1e-12,
               "zero rate sums the nominal amounts");
    check(net_present_value(flows, 0.10) < 0.0, "higher rate lowers this project's NPV");
    check_near(discount_factor(0.05, 0.0), 1.0, 0.0,
               "payment today needs no discount");
    check_near(present_value({0.0, -1000.0}, 0.05), -1000.0, 0.0,
               "up-front negative cash flow");
    check_near(present_value({2.0, -50.0}, 0.0), -50.0, 0.0,
               "negative future cash flow");
    check(present_value({1.0, 100.0}, -0.05) > 100.0,
          "negative rates can increase present value");
    check_near(net_present_value({}, 0.05), 0.0, 0.0, "empty schedule is zero");
    const std::vector<CashFlow> reverse = {
        {3.0, 400.0}, {2.0, 400.0}, {1.0, 400.0}, {0.0, -1000.0}
    };
    check_near(net_present_value(reverse, 0.05), reference, 1e-10,
               "reversing order preserves NPV within rounding tolerance");
    check_near(net_present_value(flows, 0.05), net_present_value(flows, 0.05),
               0.0, "deterministic reproducibility");
    check_near(present_value({1.0, 100.0}, 1000.0), 0.0, 0.0,
               "very small factors underflow to zero");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    check_throws<std::invalid_argument>([&] { (void)discount_factor(nan, 1.0); },
                                         "reject NaN rate");
    check_throws<std::invalid_argument>([&] { (void)discount_factor(inf, 1.0); },
                                         "reject infinite rate");
    check_throws<std::invalid_argument>([&] { (void)discount_factor(0.05, nan); },
                                         "reject NaN year");
    check_throws<std::invalid_argument>([&] { (void)discount_factor(0.05, -1.0); },
                                         "reject negative time");
    check_throws<std::invalid_argument>([&] { (void)present_value({1.0, inf}, 0.05); },
                                         "reject infinite payment");
    check_throws<std::invalid_argument>([&] { (void)net_present_value({}, nan); },
                                         "reject invalid rate even for empty schedule");
    check_throws<std::overflow_error>([&] { (void)discount_factor(-1000.0, 1.0); },
                                      "reject overflowing exponential");
    check_throws<std::overflow_error>([&] { (void)discount_factor(-1e308, 100.0); },
                                      "reject exponent multiplication overflow");
    check_throws<std::overflow_error>([&] {
        (void)present_value({1.0, std::numeric_limits<double>::max()}, -1.0);
    }, "reject present-value multiplication overflow");
    check_throws<std::overflow_error>([&] {
        (void)net_present_value({{0.0, 1e308}, {0.0, 1e308}}, 0.0);
    }, "reject result too large for double");
    std::cout << "PASS: " << checks << " checks\n";
}

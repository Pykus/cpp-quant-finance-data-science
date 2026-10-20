#include "wilson.hpp"
#include <cmath>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
namespace {
std::uint64_t checks = 0;
void check(bool passed, const char* why) {
    ++checks;
    if (!passed) throw std::runtime_error(why);
}
void near(double actual, double expected, double tolerance, const char* why) {
    check(std::fabs(actual - expected) < tolerance, why);
}
template<class F> void invalid(F f, const char* why) {
    bool threw = false;
    try { f(); } catch (const std::invalid_argument&) { threw = true; }
    check(threw, why);
}
void test_errors() {
    invalid([] { (void)statistics::wilson(0, 0); }, "zero trials");
    invalid([] { (void)statistics::wilson(2, 1); }, "successes > trials");
    invalid([] { (void)statistics::wilson(0, 1, 0); }, "zero z");
    invalid([] { (void)statistics::wilson(0, 1, -2); }, "negative z");
    invalid([] {
        (void)statistics::wilson(0, 1, std::numeric_limits<double>::quiet_NaN());
    }, "NaN z");
    invalid([] {
        (void)statistics::wilson(0, 1, std::numeric_limits<double>::infinity());
    }, "infinite z");
    invalid([] { (void)statistics::wilson(0, 1, 1e308); }, "z squared overflow");
}
void test_known_cases() {
    const auto half = statistics::wilson(50, 100);
    near(half.rate, 0.5, 1e-14, "50/100 rate");
    near(half.lower, 0.4038315303659956, 1e-10, "50/100 lower");
    near(half.upper, 0.5961684696340044, 1e-10, "50/100 upper");
    const auto none = statistics::wilson(0, 10);
    const auto all = statistics::wilson(10, 10);
    near(none.lower, 0.0, 1e-14, "0/10 lower");
    near(none.upper, 3.841458820694125 / 13.841458820694125,
         1e-9, "0/10 upper");
    near(all.lower, 1 - none.upper, 1e-12, "10/10 lower");
    near(all.upper, 1.0, 1e-14, "10/10 upper");
    near(statistics::rate_difference(statistics::wilson(60, 300),
                                     statistics::wilson(36, 300)),
         0.08, 1e-14, "two campaigns");
}
void test_properties() {
    for (std::uint64_t n : {1ULL, 2ULL, 5ULL, 10ULL, 100ULL, 1000ULL}) {
        for (std::uint64_t successes = 0; successes <= n; ++successes) {
            const auto x = statistics::wilson(successes, n);
            check(x.trials == n && x.successes == successes, "keep counts");
            check(std::isfinite(x.rate) && std::isfinite(x.lower)
                  && std::isfinite(x.upper), "finite values");
            check(x.lower >= 0.0 && x.upper <= 1.0, "probability bounds");
            check(x.lower <= x.rate + 1e-14, "rate >= lower");
            check(x.rate <= x.upper + 1e-14, "rate <= upper");
            check(x.width() >= 0.0, "nonnegative width");
            const auto opposite = statistics::wilson(n - successes, n);
            near(x.lower + opposite.upper, 1.0, 1e-12, "complement symmetry");
        }
    }
    check(statistics::wilson(500, 1000).width()
          < statistics::wilson(50, 100).width(), "more data narrows interval");
    check(statistics::wilson(50, 100, 2.576).width()
          > statistics::wilson(50, 100, 1.645).width(), "higher z wider");
    const auto huge = statistics::wilson(500000000000ULL, 1000000000000ULL);
    check(std::isfinite(huge.lower) && std::isfinite(huge.upper),
          "large count finite");
}
}
int main() {
    try { test_errors(); test_known_cases(); test_properties(); }
    catch (const std::exception& e) {
        std::cerr << "FAIL at check " << checks << ": " << e.what() << '\n';
        return 1;
    }
    std::cout << "PASS: " << checks << " Wilson interval checks\n";
}

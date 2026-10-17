#include "bootstrap.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
int failures = 0;

void check(bool ok, const char* label) {
    ++checks;
    if (!ok) {
        ++failures;
        std::cerr << "FAILED: " << label << "\n";
    }
}

void near(double actual, double expected, double tolerance, const char* label) {
    check(std::abs(actual - expected) <= tolerance, label);
}

template <typename Exception, typename Function>
void throws(Function&& function, const char* label) {
    bool caught = false;
    try {
        function();
    } catch (const Exception&) {
        caught = true;
    }
    check(caught, label);
}
}  // namespace

int main() {
    const std::vector<double> values{1, 2, 3, 4, 5};
    near(stats::mean(values), 3.0, 1e-12, "mean of 1..5");
    near(stats::median(values), 3.0, 1e-12, "odd median");
    near(stats::median({1, 2, 10, 20}), 6.0, 1e-12, "even median");
    near(stats::percentile(values, 0.0), 1.0, 1e-12, "percentile 0");
    near(stats::percentile(values, 1.0), 5.0, 1e-12, "percentile 1");
    near(stats::percentile(values, 0.25), 2.0, 1e-12, "percentile 25");
    near(stats::percentile(values, 0.375), 2.5, 1e-12, "interpolated percentile");

    const auto first = stats::bootstrap_interval(
        values, stats::Statistic::mean, 2000, 0.95, 1234);
    const auto second = stats::bootstrap_interval(
        values, stats::Statistic::mean, 2000, 0.95, 1234);
    near(first.point_estimate, 3.0, 1e-12, "observed mean");
    check(first.lower == second.lower, "seed reproduces mean lower limit");
    check(first.upper == second.upper, "seed reproduces mean upper limit");
    check(first.lower <= first.point_estimate, "lower below point");
    check(first.upper >= first.point_estimate, "upper above point");
    check(first.lower >= 1.0, "lower inside original range");
    check(first.upper <= 5.0, "upper inside original range");
    near(first.confidence, 0.95, 1e-12, "confidence stored");
    check(first.replicates == 2000, "replicate count stored");

    const auto median = stats::bootstrap_interval(
        values, stats::Statistic::median, 2000, 0.90, 999);
    near(median.point_estimate, 3.0, 1e-12, "observed median");
    check(median.lower <= median.upper, "median interval ordered");
    check(median.lower >= 1.0, "median lower bound");
    check(median.upper <= 5.0, "median upper bound");

    const auto identical = stats::bootstrap_interval(
        {7, 7, 7, 7}, stats::Statistic::mean, 300, 0.95, 5);
    near(identical.point_estimate, 7.0, 1e-12, "constant point");
    near(identical.lower, 7.0, 1e-12, "constant lower");
    near(identical.upper, 7.0, 1e-12, "constant upper");

    const auto identical_median = stats::bootstrap_interval(
        {7, 7}, stats::Statistic::median, 100, 0.95, 9);
    near(identical_median.lower, 7.0, 1e-12, "two-point lower");
    near(identical_median.upper, 7.0, 1e-12, "two-point upper");

    throws<std::invalid_argument>([] { (void)stats::mean({}); }, "empty mean");
    throws<std::invalid_argument>([] { (void)stats::median({}); }, "empty median");
    throws<std::invalid_argument>([] { (void)stats::percentile({}, 0.5); },
                                   "empty percentile");
    throws<std::invalid_argument>([] { (void)stats::percentile({2, 1}, 0.5); },
                                   "unsorted percentile");
    throws<std::invalid_argument>([] { (void)stats::percentile({1, 2}, -0.1); },
                                   "negative percentile");
    throws<std::invalid_argument>([] { (void)stats::percentile({1, 2}, 1.1); },
                                   "percentile above one");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1}, stats::Statistic::mean);
    }, "one observation");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1, 2}, stats::Statistic::mean, 99);
    }, "too few resamples");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1, 2}, stats::Statistic::mean, 100, 1.0);
    }, "confidence one");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1, 2}, stats::Statistic::mean, 100, 0.0);
    }, "confidence zero");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1, std::numeric_limits<double>::quiet_NaN()},
                                  stats::Statistic::mean);
    }, "reject NaN");
    throws<std::invalid_argument>([] {
        (void)stats::bootstrap_interval({1, std::numeric_limits<double>::infinity()},
                                  stats::Statistic::median);
    }, "reject infinity");

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}

#include "standard_error.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

int checks = 0;

void near(double actual, double expected, double tolerance, const std::string& label) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance) {
        throw std::runtime_error("FAILED: " + label);
    }
    ++checks;
}

void equal(std::size_t actual, std::size_t expected, const std::string& label) {
    if (actual != expected) {
        throw std::runtime_error("FAILED: " + label);
    }
    ++checks;
}

template <class Exception, class Action>
void throws_as(Action action, const std::string& label) {
    try {
        action();
    } catch (const Exception&) {
        ++checks;
        return;
    }
    throw std::runtime_error("Expected exception: " + label);
}

}  // namespace

int main() {
    try {
        const auto base = statistics::summarize_mean({8, 10, 12, 14, 16});
        equal(base.sample_size, 5, "n=5");
        near(base.mean, 12.0, 1e-12, "mean");
        near(base.sample_standard_deviation, std::sqrt(10.0), 1e-12, "sample SD");
        near(base.standard_error, std::sqrt(2.0), 1e-12, "standard error");

        const auto pair = statistics::summarize_mean({10, 14});
        equal(pair.sample_size, 2, "n=2");
        near(pair.mean, 12.0, 1e-12, "n=2 mean");
        near(pair.sample_standard_deviation, std::sqrt(8.0), 1e-12, "n=2 SD");
        near(pair.standard_error, 2.0, 1e-12, "n=2 SE");

        const auto constant = statistics::summarize_mean({5, 5, 5, 5});
        near(constant.mean, 5.0, 1e-12, "constant mean");
        near(constant.sample_standard_deviation, 0.0, 1e-12, "constant SD");
        near(constant.standard_error, 0.0, 1e-12, "constant SE");

        const auto signed_values = statistics::summarize_mean({-2, 0, 2});
        near(signed_values.mean, 0.0, 1e-12, "negative values mean");
        near(signed_values.sample_standard_deviation, 2.0, 1e-12, "negative values SD");
        near(signed_values.standard_error, 2.0 / std::sqrt(3.0), 1e-12,
             "negative values SE");

        const auto shifted = statistics::summarize_mean({108, 110, 112, 114, 116});
        near(shifted.mean, 112.0, 1e-12, "translation mean");
        near(shifted.standard_error, base.standard_error, 1e-12, "translation SE");
        const auto scaled = statistics::summarize_mean({16, 20, 24, 28, 32});
        near(scaled.mean, 24.0, 1e-12, "scaled mean");
        near(scaled.sample_standard_deviation, 2.0 * base.sample_standard_deviation,
             1e-12, "scaled SD");
        near(scaled.standard_error, 2.0 * base.standard_error, 1e-12, "scaled SE");

        const auto large_offset =
            statistics::summarize_mean({1e9, 1e9 + 2.0, 1e9 + 4.0});
        near(large_offset.mean, 1e9 + 2.0, 1e-6, "stable large-offset mean");
        near(large_offset.sample_standard_deviation, 2.0, 1e-6,
             "stable large-offset SD");

        throws_as<std::invalid_argument>(
            [] { (void)statistics::summarize_mean({}); }, "empty sample");
        throws_as<std::invalid_argument>(
            [] { (void)statistics::summarize_mean({7}); }, "one observation");
        throws_as<std::invalid_argument>([] {
            (void)statistics::summarize_mean(
                {1, std::numeric_limits<double>::quiet_NaN()});
        }, "NaN rejected");
        throws_as<std::invalid_argument>([] {
            (void)statistics::summarize_mean(
                {1, std::numeric_limits<double>::infinity()});
        }, "infinity rejected");
        throws_as<std::invalid_argument>([] {
            (void)statistics::summarize_mean(
                {-std::numeric_limits<double>::infinity(), 1});
        }, "negative infinity rejected");
        throws_as<std::overflow_error>([] {
            (void)statistics::summarize_mean(
                {std::numeric_limits<double>::max(),
                 -std::numeric_limits<double>::max()});
        }, "numeric overflow rejected");

        std::cout << "PASS: " << checks << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

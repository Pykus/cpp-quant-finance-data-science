#include "z_scores.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;

void expect_near(const double actual, const double expected,
                 const double tolerance, const std::string& label) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance) {
        throw std::runtime_error("Failed: " + label);
    }
    ++checks;
}

void expect_true(const bool actual, const std::string& label) {
    if (!actual) {
        throw std::runtime_error("Failed: " + label);
    }
    ++checks;
}

template <typename Exception, typename Operation>
void expect_throws(Operation operation, const std::string& label) {
    try {
        operation();
    } catch (const Exception&) {
        ++checks;
        return;
    }
    throw std::runtime_error("Expected an exception: " + label);
}
}  // namespace

int main() {
    try {
        const std::vector<double> observations{10, 12, 14, 16, 18};
        const auto summary = statistics::describe_sample(observations);
        const auto scores = statistics::z_scores(observations);
        const double root_ten = std::sqrt(10.0);

        expect_near(summary.mean, 14.0, 1e-12, "sample mean");
        expect_near(summary.sample_standard_deviation, root_ten, 1e-12, "sample sd");
        expect_true(scores.size() == observations.size(), "output size");
        expect_near(scores.at(0), -4.0 / root_ten, 1e-12, "first z-score");
        expect_near(scores.at(1), -2.0 / root_ten, 1e-12, "second z-score");
        expect_near(scores.at(2), 0.0, 1e-12, "center z-score");
        expect_near(scores.at(3), 2.0 / root_ten, 1e-12, "fourth z-score");
        expect_near(scores.at(4), 4.0 / root_ten, 1e-12, "fifth z-score");

        double score_sum = 0.0;
        for (const double score : scores) {
            score_sum += score;
        }
        expect_near(score_sum, 0.0, 1e-12, "standardized mean zero");
        const auto standardized_summary = statistics::describe_sample(scores);
        expect_near(standardized_summary.mean, 0.0, 1e-12, "computed standardized mean");
        expect_near(standardized_summary.sample_standard_deviation, 1.0, 1e-12,
                    "computed standardized sample sd");

        const auto shifted = statistics::z_scores({110, 112, 114, 116, 118});
        const auto scaled = statistics::z_scores({20, 24, 28, 32, 36});
        const auto reversed = statistics::z_scores({-10, -12, -14, -16, -18});
        for (std::size_t index = 0; index < scores.size(); ++index) {
            expect_near(shifted[index], scores[index], 1e-12, "shift invariance");
            expect_near(scaled[index], scores[index], 1e-12, "positive scale invariance");
            expect_near(reversed[index], -scores[index], 1e-12,
                        "negative scale reverses signs");
        }

        const auto two = statistics::z_scores({2, 4});
        expect_near(two.at(0), -1.0 / std::sqrt(2.0), 1e-12, "n=2 first");
        expect_near(two.at(1), 1.0 / std::sqrt(2.0), 1e-12, "n=2 second");
        const auto negative = statistics::describe_sample({-2, 0, 2});
        expect_near(negative.mean, 0.0, 1e-12, "negative numbers permitted");
        expect_near(negative.sample_standard_deviation, 2.0, 1e-12,
                    "sd with negative values");

        expect_throws<std::invalid_argument>([] { (void)statistics::z_scores({}); },
                                             "empty input");
        expect_throws<std::invalid_argument>([] { (void)statistics::z_scores({1}); },
                                             "single input");
        expect_throws<std::domain_error>([] { (void)statistics::z_scores({5, 5, 5}); },
                                         "constant input");
        expect_throws<std::invalid_argument>([] {
            (void)statistics::z_scores({1, std::numeric_limits<double>::infinity()});
        }, "infinite input");
        expect_throws<std::invalid_argument>([] {
            (void)statistics::z_scores({1, std::numeric_limits<double>::quiet_NaN()});
        }, "nan input");
        expect_throws<std::invalid_argument>([] {
            (void)statistics::describe_sample({std::numeric_limits<double>::infinity(), 1});
        }, "invalid input in summary");

        std::cout << "PASS: " << checks << " checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

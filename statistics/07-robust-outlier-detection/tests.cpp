#include "robust.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;
void check(bool ok, const char* label) {
    ++checks;
    if (!ok) throw std::runtime_error(label);
}
void near(double actual, double expected, double epsilon, const char* label) {
    check(std::abs(actual - expected) <= epsilon, label);
}
template <typename Error, typename Function>
void expect_error(Function fn, const char* label) {
    bool caught = false;
    try { fn(); } catch (const Error&) { caught = true; }
    check(caught, label);
}

void run() {
    using statistics::detect_outliers;
    using statistics::median_of;

    near(median_of({1.0}), 1.0, 1e-12, "single median");
    near(median_of({3.0, 1.0, 2.0}), 2.0, 1e-12, "odd median");
    near(median_of({4.0, 1.0, 2.0, 3.0}), 2.5, 1e-12, "even median");
    near(median_of({-4.0, 0.0}), -2.0, 1e-12, "negative median");
    const double near_max = std::numeric_limits<double>::max();
    check(std::isfinite(median_of({near_max, near_max})), "huge median no overflow");

    const std::vector<double> sample{9.8, 10.0, 10.1, 10.2, 9.9, 10.0, 35.0};
    const auto report = detect_outliers(sample);
    near(report.median, 10.0, 1e-12, "sensor median");
    near(report.median_absolute_deviation, 0.1, 1e-12, "sensor MAD");
    check(report.scores_defined, "scores defined when MAD positive");
    check(report.modified_z_scores.size() == sample.size(), "one score per input");
    check(report.flagged_indices.size() == 1, "only one outlier");
    check(report.flagged_indices[0] == 6, "last reading is outlier");
    check(report.modified_z_scores[6].has_value(), "score exists");
    check(*report.modified_z_scores[6] > 100.0, "large positive outlier score");
    check(*report.modified_z_scores[0] < 0.0, "negative residual gives negative score");
    near(*report.modified_z_scores[1], 0.0, 1e-12, "center score zero");
    check(sample.back() == 35.0, "input is not changed");

    const std::vector<double> flat{5.0, 5.0, 5.0, 5.0, 99.0};
    const auto degenerate = detect_outliers(flat);
    check(degenerate.median_absolute_deviation == 0.0, "flat MAD zero");
    check(!degenerate.scores_defined, "zero MAD reported");
    check(degenerate.flagged_indices.empty(), "zero MAD does not invent z scores");
    for (const auto& score : degenerate.modified_z_scores) {
        check(!score.has_value(), "no fake zero scores");
    }

    const auto negative = detect_outliers(std::vector<double>{-35.0, -10.2, -10.1, -10.0, -9.9, -10.0, -9.8});
    check(negative.flagged_indices.size() == 1, "negative outlier caught");
    check(negative.flagged_indices[0] == 0, "negative outlier row index");
    check(*negative.modified_z_scores[0] < -100.0, "negative score is negative");

    const auto permissive = detect_outliers(sample, 1000.0);
    check(permissive.flagged_indices.empty(), "raising threshold suppresses flags");
    const auto strict = detect_outliers(sample, 0.5);
    check(strict.flagged_indices.size() > 1, "lowering threshold adds flags");

    const std::vector<double> no_outliers{10.0, 10.1, 9.9, 10.2, 9.8};
    const auto normal = detect_outliers(no_outliers);
    check(normal.flagged_indices.empty(), "ordinary readings not flagged");
    near(normal.median, 10.0, 1e-12, "symmetric series center");

    // Translation invariance: adding a constant must not change scores/flags.
    std::vector<double> shifted = sample;
    for (double& value : shifted) value += 1000.0;
    const auto moved = detect_outliers(shifted);
    near(moved.median, 1010.0, 1e-12, "shifted median");
    check(moved.flagged_indices == report.flagged_indices, "translation preserves outliers");
    for (std::size_t i = 0; i < sample.size(); ++i) {
        near(*report.modified_z_scores[i], *moved.modified_z_scores[i],
             1e-8, "translation preserves modified z");
    }

    // Scale invariance for positive factors.
    std::vector<double> scaled = sample;
    for (double& value : scaled) value *= 2.0;
    const auto twice = detect_outliers(scaled);
    check(twice.flagged_indices == report.flagged_indices, "positive scale preserves flags");
    for (std::size_t i = 0; i < sample.size(); ++i) {
        near(*report.modified_z_scores[i], *twice.modified_z_scores[i],
             1e-10, "scale preserves modified z");
    }

    expect_error<std::invalid_argument>([]{ (void)detect_outliers(std::vector<double>{}); }, "empty");
    expect_error<std::invalid_argument>([]{ (void)detect_outliers(std::vector<double>{1.0, 2.0}); }, "too few");
    expect_error<std::invalid_argument>([&]{ (void)detect_outliers(sample, 0.0); }, "zero threshold");
    expect_error<std::invalid_argument>([&]{ (void)detect_outliers(sample, -1.0); }, "negative threshold");
    expect_error<std::invalid_argument>([&]{ (void)detect_outliers(sample, std::numeric_limits<double>::quiet_NaN()); }, "nan threshold");
    expect_error<std::invalid_argument>([&]{ (void)detect_outliers(sample, std::numeric_limits<double>::infinity()); }, "infinite threshold");
    expect_error<std::invalid_argument>([]{ (void)median_of({}); }, "empty median");
    expect_error<std::invalid_argument>([]{ (void)median_of({1.0, std::numeric_limits<double>::quiet_NaN()}); }, "nan median");
    expect_error<std::invalid_argument>([]{
        (void)detect_outliers(std::vector<double>{1.0, std::numeric_limits<double>::infinity(), 2.0});
    }, "infinite observation");
    expect_error<std::overflow_error>([&]{
        (void)detect_outliers(std::vector<double>{-near_max, -near_max, near_max});
    }, "overflowing absolute deviation");

    // Repeatable deterministic screening of a synthetic 100-element series.
    std::vector<double> long_sample;
    for (int i = 0; i < 99; ++i) long_sample.push_back(10.0 + (i % 5) * 0.1);
    long_sample.push_back(80.0);
    const auto many = detect_outliers(long_sample);
    check(many.flagged_indices.size() == 1, "one injected anomaly in 100");
    check(many.flagged_indices[0] == 99, "injected anomaly indexed correctly");
}
}

int main() {
    try {
        run();
        std::cout << checks << " checks passed\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}

#include "risk.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
    ++checks;
}
void near(double actual, double expected, const std::string& label) {
    check(std::abs(actual - expected) < 1e-12, label);
}
template <typename Work>
void rejects(Work work, const std::string& label) {
    try {
        work();
    } catch (const std::invalid_argument&) {
        ++checks;
        return;
    }
    throw std::runtime_error(label);
}
} // namespace

int main() {
    try {
        const std::vector<double> a{0.10, -0.02, 0.06};
        const std::vector<double> b{0.00, 0.04, 0.02};
        const RiskSummary risk = analyze_portfolio(a, b, 0.60);
        near(risk.mean_a, 0.14 / 3.0, "mean A");
        near(risk.mean_b, 0.02, "mean B");
        near(risk.mean_portfolio, 0.036, "mean portfolio");
        near(risk.variance_a, 0.003733333333333333, "sample variance A");
        near(risk.variance_b, 0.0004, "sample variance B");
        near(risk.covariance, -0.0012, "sample covariance");
        near(risk.variance_portfolio, 0.000832, "sample variance portfolio");
        near(risk.standard_deviation, std::sqrt(0.000832), "sample standard deviation");
        near(risk.variance_portfolio,
             0.60 * 0.60 * risk.variance_a +
             0.40 * 0.40 * risk.variance_b +
             2.0 * 0.60 * 0.40 * risk.covariance,
             "variance identity");

        const RiskSummary opposite = analyze_portfolio({0.1, -0.1}, {-0.1, 0.1}, 0.5);
        near(opposite.standard_deviation, 0.0, "perfect offsets cancel");
        near(opposite.covariance, -0.02, "opposite covariance");

        const RiskSummary first_only = analyze_portfolio(a, b, 1.0);
        near(first_only.variance_portfolio, first_only.variance_a, "100% asset A");
        const RiskSummary second_only = analyze_portfolio(a, b, 0.0);
        near(second_only.variance_portfolio, second_only.variance_b, "100% asset B");

        rejects([&] { (void)analyze_portfolio({0.1}, {0.2}, 0.5); }, "too few rows");
        rejects([&] { (void)analyze_portfolio(a, {0.1, 0.2}, 0.5); }, "mismatched rows");
        rejects([&] { (void)analyze_portfolio(a, b, -0.01); }, "negative weight");
        rejects([&] { (void)analyze_portfolio(a, b, 1.01); }, "weight above one");
        rejects([&] { (void)analyze_portfolio(a, b, std::numeric_limits<double>::quiet_NaN()); }, "NaN weight");
        rejects([&] { (void)analyze_portfolio({0.0, std::numeric_limits<double>::infinity()}, {0.0, 0.0}, 0.5); }, "infinite return");
        rejects([&] { (void)analyze_portfolio({0.0, 0.0}, {0.0, std::numeric_limits<double>::quiet_NaN()}, 0.5); }, "NaN return");
        rejects([&] { (void)analyze_portfolio({1e308, 1e308}, {0.0, 0.0}, 0.5); }, "overflowed accumulation");
        std::cout << checks << " checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}

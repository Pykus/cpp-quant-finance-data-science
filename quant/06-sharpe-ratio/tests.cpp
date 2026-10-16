#include "sharpe.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0;
void check(bool condition, const std::string& name) {
    if (!condition) {
        throw std::runtime_error("Failed: " + name);
    }
    ++checks;
}
void near(double got, double expected, const std::string& name,
          double tolerance = 1e-12) {
    check(std::isfinite(got) && std::abs(got - expected) <= tolerance, name);
}
template <typename Exception, typename Action>
void rejects(Action action, const std::string& name) {
    try {
        action();
    } catch (const Exception&) {
        ++checks;
        return;
    }
    throw std::runtime_error("Expected exception: " + name);
}
} // namespace

int main() {
    try {
        const std::vector<double> a{0.04, -0.02, 0.06, 0.00, 0.01};
        const std::vector<double> b{0.00, 0.00, 0.00, 0.00, 0.01};
        const auto returns = lesson06::two_asset_returns(a, b, 0.5);
        check(returns.size() == 5, "five combined periods");
        const std::vector<double> expected{0.02, -0.01, 0.03, 0.00, 0.01};
        for (std::size_t i = 0; i < returns.size(); ++i) {
            near(returns[i], expected[i], "portfolio return period " + std::to_string(i));
        }

        const auto result = lesson06::analyze_sharpe(returns, 0.002);
        check(result.observations == 5, "count");
        near(result.mean_return, 0.01, "mean return");
        near(result.risk_free_return, 0.002, "risk-free return");
        near(result.mean_excess_return, 0.008, "mean excess return");
        near(result.sample_variance, 0.00025, "sample variance");
        near(result.sample_standard_deviation, std::sqrt(0.00025), "sample sd");
        near(result.sharpe_ratio, 0.008 / std::sqrt(0.00025), "Sharpe ratio");

        const auto all_a = lesson06::two_asset_returns(a, b, 1.0);
        const auto all_b = lesson06::two_asset_returns(a, b, 0.0);
        for (std::size_t i = 0; i < a.size(); ++i) {
            near(all_a[i], a[i], "100 percent A");
            near(all_b[i], b[i], "100 percent B");
        }
        const auto small = lesson06::analyze_sharpe({0.03, 0.01}, 0.01);
        check(small.observations == 2, "two observations accepted");
        near(small.mean_return, 0.02, "two-month mean");
        near(small.mean_excess_return, 0.01, "two-month excess");
        near(small.sample_variance, 0.0002, "two-month sample variance");
        near(small.sample_standard_deviation, std::sqrt(0.0002), "two-month sd");
        near(small.sharpe_ratio, 0.01 / std::sqrt(0.0002), "two-month Sharpe");

        const auto zero_excess = lesson06::analyze_sharpe({0.03, 0.01}, 0.02);
        near(zero_excess.mean_excess_return, 0.0, "zero excess");
        near(zero_excess.sharpe_ratio, 0.0, "zero Sharpe");
        const auto negative = lesson06::analyze_sharpe({0.03, 0.01}, 0.03);
        check(negative.sharpe_ratio < 0, "negative Sharpe");
        near(negative.sharpe_ratio, -0.01 / std::sqrt(0.0002), "negative value");
        const auto no_risk_free = lesson06::analyze_sharpe(returns, 0.0);
        near(no_risk_free.sharpe_ratio, 0.01 / std::sqrt(0.00025), "zero risk-free");
        const auto scaled = lesson06::analyze_sharpe({0.20, -0.10, 0.30, 0.00, 0.10}, 0.02);
        near(scaled.sharpe_ratio, result.sharpe_ratio, "consistent scaling");

        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({}, 0.0); }, "empty sample");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({0.1}, 0.0); }, "single observation");
        rejects<std::domain_error>(
            [&] { (void)lesson06::analyze_sharpe({0.02, 0.02, 0.02}, 0.0); },
            "identical observations");
        rejects<std::domain_error>(
            [&] { (void)lesson06::analyze_sharpe({0.0, -0.0}, 0.0); },
            "signed zero is constant");
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        const auto inf = std::numeric_limits<double>::infinity();
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({0.01, nan}, 0.0); }, "NaN return");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({0.01, inf}, 0.0); }, "infinite return");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({0.01, 0.02}, nan); }, "NaN risk-free");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({0.01, 0.02}, inf); }, "infinite risk-free");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::analyze_sharpe({1e308, -1e308}, 0.0); },
            "unrepresentable variance");

        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns({}, {}, 0.5); }, "empty assets");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns({0.01}, {0.02}, 0.5); },
            "one asset row");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns(a, {0.01, 0.02}, 0.5); },
            "mismatched rows");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns(a, b, -0.1); }, "negative weight");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns(a, b, 1.1); }, "weight above one");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns(a, b, nan); }, "NaN weight");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns(a, b, inf); }, "infinite weight");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns({0.01, nan}, {0.02, 0.03}, 0.5); },
            "NaN in asset A");
        rejects<std::invalid_argument>(
            [&] { (void)lesson06::two_asset_returns({0.01, 0.02}, {0.03, inf}, 0.5); },
            "infinity in asset B");

        std::cout << checks << " checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#include "risk.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

struct Checks {
    int total = 0;
    int failed = 0;

    void yes(bool condition, std::string_view description) {
        ++total;
        if (!condition) {
            ++failed;
            std::cerr << "FAIL: " << description << '\n';
        }
    }
    void near(double actual, double wanted, std::string_view description) {
        yes(std::isfinite(actual) && std::abs(actual - wanted) < 1e-11,
            description);
    }
    template <typename Function>
    void invalid(Function&& function, std::string_view description) {
        ++total;
        try {
            function();
            ++failed;
            std::cerr << "FAIL: " << description << " (no exception)\n";
        } catch (const std::invalid_argument&) {
            // Correctly rejected malformed input.
        } catch (...) {
            ++failed;
            std::cerr << "FAIL: " << description << " (wrong exception)\n";
        }
    }
};

template <typename Container>
quant::TailRisk measure(const Container& data, double confidence) {
    return quant::historical_tail_risk(
        std::span<const double>{data.data(), data.size()}, confidence);
}

int main() {
    Checks test;
    const std::array data{
        -0.10, -0.06, -0.04, -0.02, 0.00,
         0.01,  0.02,  0.03,  0.04, 0.05};

    const auto r80 = measure(data, 0.80);
    test.near(r80.confidence, 0.80, "confidence retained");
    test.yes(r80.observations == 10, "observation count");
    test.near(r80.value_at_risk, 0.04, "80% nearest-rank VaR");
    test.near(r80.expected_shortfall, 0.08, "80% ES averages two worst");
    test.yes(r80.expected_shortfall >= r80.value_at_risk, "ES >= VaR");

    const auto r90 = measure(data, 0.90);
    test.near(r90.value_at_risk, 0.06, "90% VaR");
    test.near(r90.expected_shortfall, 0.10, "90% ES");

    const auto r85 = measure(data, 0.85);
    test.near(r85.value_at_risk, 0.06, "85% VaR");
    test.near(r85.expected_shortfall, (0.10 + 0.5 * 0.06) / 1.5,
              "85% fractional tail boundary");

    const auto r99 = measure(data, 0.99);
    test.near(r99.value_at_risk, 0.10, "99% small-sample VaR");
    test.near(r99.expected_shortfall, 0.10, "99% small-sample ES");

    auto reordered = data;
    std::reverse(reordered.begin(), reordered.end());
    const auto reordered_risk = measure(reordered, 0.80);
    test.near(reordered_risk.value_at_risk, r80.value_at_risk,
              "invariant under observation permutation VaR");
    test.near(reordered_risk.expected_shortfall, r80.expected_shortfall,
              "invariant under observation permutation ES");

    const std::array single{-0.12};
    const auto single_risk = measure(single, 0.95);
    test.near(single_risk.value_at_risk, 0.12, "single observation VaR");
    test.near(single_risk.expected_shortfall, 0.12, "single observation ES");
    test.yes(single_risk.observations == 1, "single observation count");

    const std::array all_zero{0.0, 0.0, 0.0, 0.0};
    test.near(measure(all_zero, 0.8).value_at_risk, 0.0, "all-zero VaR");
    test.near(measure(all_zero, 0.8).expected_shortfall, 0.0, "all-zero ES");

    const std::array all_gain{0.01, 0.02, 0.03, 0.04};
    test.near(measure(all_gain, 0.5).value_at_risk, -0.03,
              "negative VaR possible if all returns positive");
    test.near(measure(all_gain, 0.5).expected_shortfall, -0.015,
              "negative ES possible if all returns positive");

    const std::array repeated{-0.05, -0.05, -0.05, 0.01};
    test.near(measure(repeated, 0.5).value_at_risk, 0.05, "ties in VaR");
    test.near(measure(repeated, 0.5).expected_shortfall, 0.05, "ties in ES");

    const std::array complete_loss{-1.0, 0.0};
    test.near(measure(complete_loss, 0.5).expected_shortfall, 1.0,
              "100% portfolio loss is valid");
    test.near(measure(complete_loss, 0.5).value_at_risk, 0.0,
              "median after complete-loss example");

    const auto money = quant::scale_to_money(r80, 10'000.0);
    test.near(money.value_at_risk, 400.0, "money VaR");
    test.near(money.expected_shortfall, 800.0, "money ES");
    test.near(quant::scale_to_money(r80, 1.0).value_at_risk, 0.04,
              "unit portfolio value");
    test.near(quant::scale_to_money(r80, 100.0).expected_shortfall, 8.0,
              "100 unit portfolio");
    test.yes(measure(data, 0.80).expected_shortfall == r80.expected_shortfall,
             "deterministic ES");

    const std::vector<double> empty;
    test.invalid([&]{ measure(empty, 0.8); }, "empty series");
    test.invalid([&]{ measure(data, 0.0); }, "zero confidence");
    test.invalid([&]{ measure(data, 1.0); }, "unit confidence");
    test.invalid([&]{ measure(data, -0.1); }, "negative confidence");
    test.invalid([&]{ measure(data, 1.1); }, "confidence above one");
    test.invalid([&]{ measure(data, std::numeric_limits<double>::quiet_NaN()); },
                 "NaN confidence");
    test.invalid([&]{ measure(data, std::numeric_limits<double>::infinity()); },
                 "infinite confidence");

    const std::array too_low{-1.00001, 0.0};
    test.invalid([&]{ measure(too_low, 0.8); }, "return below -100%");
    const std::array nan_data{0.0, std::numeric_limits<double>::quiet_NaN()};
    test.invalid([&]{ measure(nan_data, 0.8); }, "NaN return");
    const std::array inf_data{0.0, std::numeric_limits<double>::infinity()};
    test.invalid([&]{ measure(inf_data, 0.8); }, "infinite return");
    const std::array neg_inf_data{
        0.0, -std::numeric_limits<double>::infinity()};
    test.invalid([&]{ measure(neg_inf_data, 0.8); }, "negative infinite return");

    test.invalid([&]{ (void)quant::scale_to_money(r80, 0.0); }, "zero portfolio");
    test.invalid([&]{ (void)quant::scale_to_money(r80, -1.0); }, "negative portfolio");
    test.invalid([&]{ (void)quant::scale_to_money(
        r80, std::numeric_limits<double>::quiet_NaN()); }, "NaN portfolio");
    test.invalid([&]{ (void)quant::scale_to_money(
        r80, std::numeric_limits<double>::infinity()); }, "infinite portfolio");

    std::cout << "Passed " << test.total - test.failed
              << "/" << test.total << " checks\n";
    return test.failed == 0 ? 0 : 1;
}
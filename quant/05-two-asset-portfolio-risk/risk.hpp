#ifndef TWO_ASSET_PORTFOLIO_RISK_HPP
#define TWO_ASSET_PORTFOLIO_RISK_HPP

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

// All returns and the weight are decimals: 0.10 represents 10%.
struct RiskSummary {
    double mean_a;
    double mean_b;
    double mean_portfolio;
    double variance_a;
    double variance_b;
    double covariance;
    double variance_portfolio;
    double standard_deviation;
};

// Observations must be paired, equally spaced, and cover the same periods.
// The weight is the fraction invested in A; the remainder is invested in B.
// Fixed weights mean a portfolio rebalanced to those weights each period.
[[nodiscard]] inline RiskSummary analyze_portfolio(
    const std::vector<double>& returns_a,
    const std::vector<double>& returns_b,
    double weight_a) {
    if (returns_a.size() < 2 || returns_a.size() != returns_b.size()) {
        throw std::invalid_argument("provide at least two paired observations");
    }
    if (!std::isfinite(weight_a) || weight_a < 0.0 || weight_a > 1.0) {
        throw std::invalid_argument("weight must be finite and between 0 and 1");
    }

    const double weight_b = 1.0 - weight_a;
    const std::size_t count = returns_a.size();
    std::vector<double> portfolio_returns;
    portfolio_returns.reserve(count);
    double sum_a = 0.0;
    double sum_b = 0.0;
    double sum_portfolio = 0.0;

    for (std::size_t i = 0; i < count; ++i) {
        const double a = returns_a[i];
        const double b = returns_b[i];
        if (!std::isfinite(a) || !std::isfinite(b)) {
            throw std::invalid_argument("returns must be finite");
        }
        const double p = weight_a * a + weight_b * b;
        sum_a += a;
        sum_b += b;
        sum_portfolio += p;
        portfolio_returns.push_back(p);
    }

    const double n = static_cast<double>(count);
    const double mean_a = sum_a / n;
    const double mean_b = sum_b / n;
    const double mean_portfolio = sum_portfolio / n;
    double squared_a = 0.0;
    double squared_b = 0.0;
    double paired_product = 0.0;
    double squared_portfolio = 0.0;

    for (std::size_t i = 0; i < count; ++i) {
        const double deviation_a = returns_a[i] - mean_a;
        const double deviation_b = returns_b[i] - mean_b;
        const double deviation_p = portfolio_returns[i] - mean_portfolio;
        squared_a += deviation_a * deviation_a;
        squared_b += deviation_b * deviation_b;
        paired_product += deviation_a * deviation_b;
        squared_portfolio += deviation_p * deviation_p;
    }

    const double divisor = static_cast<double>(count - 1); // Sample (not population) statistics.
    const RiskSummary result{
        mean_a,
        mean_b,
        mean_portfolio,
        squared_a / divisor,
        squared_b / divisor,
        paired_product / divisor,
        squared_portfolio / divisor,
        std::sqrt(squared_portfolio / divisor)
    };
    if (!std::isfinite(result.mean_portfolio) ||
        !std::isfinite(result.variance_a) ||
        !std::isfinite(result.variance_b) ||
        !std::isfinite(result.covariance) ||
        !std::isfinite(result.variance_portfolio) ||
        !std::isfinite(result.standard_deviation)) {
        throw std::invalid_argument("values are too large for this calculation");
    }
    return result;
}

#endif // TWO_ASSET_PORTFOLIO_RISK_HPP

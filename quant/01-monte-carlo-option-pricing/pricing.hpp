#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
namespace quant {
struct Option { double spot, strike, rate, volatility, maturity; };
struct Estimate { double price, standard_error; std::size_t paths; };
inline void validate(const Option& o) {
    if (!std::isfinite(o.spot) || !std::isfinite(o.strike) || !std::isfinite(o.rate) ||
        !std::isfinite(o.volatility) || !std::isfinite(o.maturity) ||
        o.spot <= 0 || o.strike <= 0 || o.volatility < 0 || o.maturity < 0)
        throw std::invalid_argument("invalid option parameters");
}
inline double normal_cdf(double x) { return 0.5 * std::erfc(-x / std::sqrt(2.0)); }
inline double black_scholes_call(const Option& o) {
    validate(o);
    const double discounted_strike = o.strike * std::exp(-o.rate * o.maturity);
    if (o.maturity == 0 || o.volatility == 0)
        return std::max(0.0, o.spot - discounted_strike);
    const double sigma_sqrt_t = o.volatility * std::sqrt(o.maturity);
    const double d1 = (std::log(o.spot / o.strike) +
        (o.rate + 0.5 * o.volatility * o.volatility) * o.maturity) / sigma_sqrt_t;
    const double d2 = d1 - sigma_sqrt_t;
    return o.spot * normal_cdf(d1) - discounted_strike * normal_cdf(d2);
}
inline Estimate monte_carlo_call(const Option& o, std::size_t pairs, std::uint32_t seed) {
    validate(o);
    if (pairs < 2 || pairs > 100000000)
        throw std::invalid_argument("pairs must be in [2, 100000000]");
    const double discount = std::exp(-o.rate * o.maturity);
    if (o.maturity == 0 || o.volatility == 0)
        return {black_scholes_call(o), 0.0, pairs * 2};
    std::mt19937 generator(seed);
    std::normal_distribution<double> standard_normal(0.0, 1.0);
    const double drift = (o.rate - 0.5 * o.volatility * o.volatility) * o.maturity;
    const double diffusion = o.volatility * std::sqrt(o.maturity);
    double mean = 0.0, m2 = 0.0;
    for (std::size_t i = 1; i <= pairs; ++i) {
        const double z = standard_normal(generator);
        const double up = o.spot * std::exp(drift + diffusion * z);
        const double down = o.spot * std::exp(drift - diffusion * z);
        const double sample = discount * 0.5 *
            (std::max(up - o.strike, 0.0) + std::max(down - o.strike, 0.0));
        const double delta = sample - mean;
        mean += delta / static_cast<double>(i);
        m2 += delta * (sample - mean);
    }
    const double sample_variance = m2 / static_cast<double>(pairs - 1);
    return {mean, std::sqrt(sample_variance / static_cast<double>(pairs)), pairs * 2};
}
} // namespace quant

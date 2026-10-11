#include "pricing.hpp"
#include <cassert>
#include <cmath>
#include <stdexcept>
int main() {
    const quant::Option o{100.0, 100.0, 0.05, 0.2, 1.0};
    const double analytic = quant::black_scholes_call(o);
    assert(std::abs(analytic - 10.450583572185565) < 1e-9);
    const auto a = quant::monte_carlo_call(o, 100000, 42);
    const auto b = quant::monte_carlo_call(o, 100000, 42);
    assert(a.price == b.price && a.standard_error == b.standard_error);
    assert(a.paths == 200000 && a.standard_error > 0);
    assert(std::abs(a.price - analytic) < 4 * a.standard_error);
    const quant::Option expiry{120, 100, 0.05, 0.2, 0};
    assert(quant::black_scholes_call(expiry) == 20);
    assert(quant::monte_carlo_call(expiry, 10, 7).price == 20);
    const quant::Option deterministic{100, 90, 0.05, 0, 1};
    assert(std::abs(quant::monte_carlo_call(deterministic, 10, 7).price -
                    quant::black_scholes_call(deterministic)) < 1e-12);
    bool rejected = false;
    try { quant::monte_carlo_call(o, 1, 42); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
    rejected = false;
    try { quant::black_scholes_call({-1, 100, 0.05, 0.2, 1}); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

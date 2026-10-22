#include "greeks.hpp"

#include <iomanip>
#include <iostream>

int main() {
    constexpr quant::OptionInputs inputs{
        100.0, 100.0, 0.05, 0.0, 0.20, 1.0
    };
    const auto call = quant::black_scholes(quant::OptionKind::call, inputs);
    const auto put = quant::black_scholes(quant::OptionKind::put, inputs);
    const double shares = quant::shares_for_zero_delta(
        -1.0, 100.0, call.delta);

    std::cout << std::fixed << std::setprecision(8)
              << "European call price: " << call.price << '\n'
              << "European put price:  " << put.price << '\n'
              << "Call delta:          " << call.delta << '\n'
              << "Call gamma:          " << call.gamma << '\n'
              << "Call vega:           " << call.vega << '\n'
              << "Shares to hedge one short 100-unit call: "
              << shares << '\n';
}

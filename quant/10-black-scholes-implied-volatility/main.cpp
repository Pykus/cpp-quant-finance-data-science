#include "implied_volatility.hpp"

#include <iomanip>
#include <iostream>

int main() {
    const quant::OptionInputs inputs{
        .spot = 100.0,
        .strike = 100.0,
        .rate = 0.05,
        .dividend_yield = 0.0,
        .years_to_expiry = 1.0
    };
    constexpr double observed_call_price = 10.450583572185565;
    const auto solved = quant::implied_volatility(
        quant::OptionKind::call, inputs, observed_call_price);

    std::cout << std::fixed << std::setprecision(8)
              << "Observed call price: " << observed_call_price << '\n'
              << "Implied annual volatility: " << solved.volatility << '\n'
              << "Repriced option: "
              << quant::black_scholes_price(quant::OptionKind::call, inputs,
                                           solved.volatility) << '\n'
              << "Bisection iterations: " << solved.iterations << '\n'
              << "Call vega (per unit volatility): "
              << quant::black_scholes_vega(inputs, solved.volatility) << '\n';
}

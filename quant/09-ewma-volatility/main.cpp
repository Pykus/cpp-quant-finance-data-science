#include "ewma.hpp"
#include <array>
#include <iomanip>
#include <iostream>

int main() {
    constexpr std::array<double, 6> returns{0.01,-0.02,0.015,-0.03,0.005,0.012};
    quant::EwmaVolatility model{0.94, 0.0001};
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Day | Return | Forecast sigma before | Forecast sigma after\n";
    for (std::size_t i = 0; i < returns.size(); ++i) {
        const auto before = model.forecast_volatility();
        model.observe(returns[i]);
        std::cout << i+1 << " | " << returns[i] << " | " << before
                  << " | " << model.forecast_volatility() << '\n';
    }
    const auto risk = quant::normal_tail_risk_95(model.forecast_volatility(),10000);
    std::cout << "Next-day variance: " << model.forecast_variance() << '\n';
    std::cout << "Next-day volatility: " << model.forecast_volatility() << '\n';
    std::cout << "Annualized volatility: " << model.annualized_volatility(252) << '\n';
    std::cout << "Weight half-life: " << model.half_life_observations() << '\n';
    std::cout << "Normal-model 95% VaR on 10000: " << risk.value_at_risk_amount << '\n';
    std::cout << "Normal-model 95% ES on 10000: " << risk.expected_shortfall_amount << '\n';
}

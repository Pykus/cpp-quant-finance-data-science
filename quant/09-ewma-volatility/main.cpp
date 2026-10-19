#include "ewma.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Five fictional daily portfolio returns; +0.01 represents +1%.
    const std::vector<double> returns{0.01, -0.02, 0.03, -0.01, 0.0};
    const auto snapshots = quant::ewma_risk(returns, 0.94, 100'000.0);

    std::cout << "day,ewma_daily_vol,annualized_vol,next_day_var95_usd\n"
              << std::fixed << std::setprecision(6);
    for (const auto& risk : snapshots) {
        std::cout << risk.day << ',' << risk.daily_volatility << ','
                  << risk.annualized_volatility << ','
                  << risk.next_day_var95 << '\n';
    }
}

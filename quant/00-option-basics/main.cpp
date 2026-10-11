#include <algorithm>
#include <iostream>

double call_payoff(double future_price, double strike) {
    return std::max(future_price - strike, 0.0);
}

int main() {
    const double strike = 100.0;
    for (const double future_price : {90.0, 100.0, 120.0, 150.0}) {
        std::cout << "future_price=" << future_price
                  << " strike=" << strike
                  << " payoff=" << call_payoff(future_price, strike) << '\n';
    }
}

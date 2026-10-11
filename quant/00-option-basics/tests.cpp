#include <algorithm>
#include <cassert>

double call_payoff(double future_price, double strike) {
    return std::max(future_price - strike, 0.0);
}

int main() {
    assert(call_payoff(90, 100) == 0);
    assert(call_payoff(100, 100) == 0);
    assert(call_payoff(120, 100) == 20);
    assert(call_payoff(150, 100) == 50);
}

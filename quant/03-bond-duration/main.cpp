#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

double bond_price(double face, double coupon_rate, double yield, int years) {
    if (face <= 0 || coupon_rate < 0 || yield <= -1 || years <= 0)
        throw std::invalid_argument("invalid bond parameters");
    double price = 0;
    for (int t = 1; t <= years; ++t) {
        double cashflow = face * coupon_rate + (t == years ? face : 0);
        price += cashflow / std::pow(1 + yield, t);
    }
    return price;
}

double macaulay_duration(double face, double coupon_rate, double yield, int years) {
    const double price = bond_price(face, coupon_rate, yield, years);
    double weighted = 0;
    for (int t = 1; t <= years; ++t) {
        double cashflow = face * coupon_rate + (t == years ? face : 0);
        weighted += t * cashflow / std::pow(1 + yield, t);
    }
    return weighted / price;
}

#ifndef BOND_TEST
int main() {
    std::cout << std::fixed << std::setprecision(4)
              << "price=" << bond_price(1000, 0.05, 0.06, 3) << '\n'
              << "duration=" << macaulay_duration(1000, 0.05, 0.06, 3) << '\n';
}
#endif

#include "bond_yield.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
int checks = 0;
void expect(bool ok, const char* name) {
    ++checks;
    if (!ok) { std::cerr << "FAIL " << name << '\n'; std::exit(EXIT_FAILURE); }
}
void near(double got, double want, double tol, const char* name) {
    expect(std::abs(got - want) <= tol, name);
}
template<typename E,typename Fn>
void throws(Fn&& f) {
    bool ok = false;
    try { f(); } catch (const E&) { ok = true; }
    expect(ok, "exception");
}
}

int main() {
    using quant15::AnnualBond;
    using quant15::bond_price;
    using quant15::yield_to_maturity;
    const AnnualBond par{100.0, 0.05, 3};
    near(bond_price(par, 0.05), 100.0, 1e-10, "bond at par");
    near(yield_to_maturity(par, 100.0), 0.05, 1e-10, "par yield");
    near(bond_price({100.0, 0.0, 1}, 0.05), 100.0/1.05, 1e-10, "zero coupon");
    near(yield_to_maturity({100.0, 0.0, 1}, 100.0/1.05), 0.05, 1e-10, "zero coupon yield");
    expect(bond_price(par, 0.1) < bond_price(par, 0.05), "monotonic price");
    near(yield_to_maturity(par, bond_price(par, 0.0)), 0.0, 1e-12, "zero yield");
    throws<std::invalid_argument>([] { (void)bond_price({0,0.05,2}, 0.05); });
    throws<std::invalid_argument>([] { (void)bond_price({100,0.05,2}, -0.01); });
    throws<std::invalid_argument>([] { (void)yield_to_maturity({100,0.05,2}, 0); });
    throws<std::domain_error>([] { (void)yield_to_maturity({100,0.05,3}, 200); });
    throws<std::invalid_argument>([] {
        (void)yield_to_maturity({100,0.05,3}, std::numeric_limits<double>::quiet_NaN());
    });
    for (int i = 0; i != 5000; ++i) {
        const AnnualBond b{100.0 + (i % 300), (i % 7) * 0.01, 1 + i % 20};
        const double known_yield = 0.005 + (i % 200) * 0.0005;
        const double price = bond_price(b, known_yield);
        const double recovered = yield_to_maturity(b, price, 1e-10);
        near(recovered, known_yield, 1e-8, "recovered synthetic yield");
    }
    std::cout << "PASS " << checks << " checks\n";
}

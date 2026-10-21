#include "binomial.hpp"
#include <iomanip>
#include <iostream>

int main() {
    using namespace quant::binomial;
    Contract c{100.0, 100.0, 1.0, 0.05, 0.0, 0.20,
               400, OptionKind::call, ExerciseStyle::european};
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "European call:      " << price(c) << '\n';
    std::cout << "Black-Scholes call: " << black_scholes_european(c) << '\n';
    c.kind = OptionKind::put;
    std::cout << "European put:       " << price(c) << '\n';
    c.exercise = ExerciseStyle::american;
    std::cout << "American put:       " << price(c) << '\n';
}

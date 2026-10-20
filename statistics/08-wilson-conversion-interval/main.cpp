#include "wilson.hpp"
#include <iomanip>
#include <iostream>
int main() {
    const auto a = statistics::wilson(36, 300);
    const auto b = statistics::wilson(60, 300);
    std::cout << std::fixed << std::setprecision(2);
    const auto print = [](const char* name, const statistics::Interval& x) {
        std::cout << name << ": " << x.successes << '/' << x.trials
                  << " conversions, observed " << 100 * x.rate
                  << "%, 95% Wilson interval [" << 100 * x.lower << "%, "
                  << 100 * x.upper << "%]\n";
    };
    print("Campaign A", a);
    print("Campaign B", b);
    std::cout << "Observed difference (B-A): "
              << 100 * statistics::rate_difference(b, a)
              << " percentage points; NOT a significance test.\n";
}

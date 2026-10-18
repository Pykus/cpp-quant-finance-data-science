#include "bootstrap_mean.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
std::size_t passed = 0;
void check(bool value, const char* name) {
    if (!value) throw std::runtime_error(std::string("FAILED: ") + name);
    ++passed;
}
void near(double a, double b, double tolerance, const char* name) {
    check(std::abs(a-b) <= tolerance, name);
}
template<typename Function> void rejects(Function f, const char* name) {
    bool rejected = false;
    try { f(); } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected, name);
}
} // namespace

int main() {
    try {
        using statistics::bootstrap_mean_interval;
        const std::vector<double> x{8,10,12,14,16};
        const auto a = bootstrap_mean_interval(x, 4000, 0.95, 42);
        const auto b = bootstrap_mean_interval(x, 4000, 0.95, 42);
        near(a.sample_mean, 12, 0.0, "mean");
        check(a.resamples == 4000, "count");
        check(a.lower < 12 && a.upper > 12, "CI surrounds mean");
        check(a.lower >= 8 && a.upper <= 16, "bounds in range");
        check(a.bootstrap_standard_error > 0.9, "SE lower");
        check(a.bootstrap_standard_error < 1.7, "SE upper");
        near(a.lower, b.lower, 0.0, "deterministic lower");
        near(a.upper, b.upper, 0.0, "deterministic upper");
        near(a.bootstrap_standard_error, b.bootstrap_standard_error, 0.0, "deterministic SE");
        const auto narrow = bootstrap_mean_interval(x, 4000, 0.80, 42);
        check(narrow.lower >= a.lower, "80 CI lower inside 95 CI");
        check(narrow.upper <= a.upper, "80 CI upper inside 95 CI");
        const auto constant = bootstrap_mean_interval({7,7,7,7}, 100, 0.95, 2);
        near(constant.sample_mean, 7, 0, "constant mean");
        near(constant.lower, 7, 0, "constant lower");
        near(constant.upper, 7, 0, "constant upper");
        near(constant.bootstrap_standard_error, 0, 0, "constant SE");
        const auto shifted = bootstrap_mean_interval({108,110,112,114,116}, 4000, 0.95, 42);
        near(shifted.sample_mean, 112, 0, "shift mean");
        near(shifted.lower, a.lower+100, 1e-9, "shift low");
        near(shifted.upper, a.upper+100, 1e-9, "shift high");
        near(shifted.bootstrap_standard_error, a.bootstrap_standard_error, 1e-9, "shift SE");
        const auto doubled = bootstrap_mean_interval({16,20,24,28,32}, 4000, 0.95, 42);
        near(doubled.lower, 2*a.lower, 1e-9, "double low");
        near(doubled.upper, 2*a.upper, 1e-9, "double high");
        near(doubled.bootstrap_standard_error, 2*a.bootstrap_standard_error, 1e-9, "double SE");
        rejects([&]{(void)bootstrap_mean_interval({},100,0.95);}, "empty");
        rejects([&]{(void)bootstrap_mean_interval({1},100,0.95);}, "singleton");
        rejects([&]{(void)bootstrap_mean_interval(x,99,0.95);}, "too few draws");
        rejects([&]{(void)bootstrap_mean_interval(x,10'000'001,0.95);}, "too many draws");
        rejects([&]{(void)bootstrap_mean_interval(x,100,0.0);}, "zero confidence");
        rejects([&]{(void)bootstrap_mean_interval(x,100,1.0);}, "full confidence");
        rejects([&]{(void)bootstrap_mean_interval(x,100,-1.0);}, "negative confidence");
        rejects([&]{(void)bootstrap_mean_interval(x,100,std::numeric_limits<double>::quiet_NaN());}, "NaN confidence");
        rejects([&]{(void)bootstrap_mean_interval({1,std::numeric_limits<double>::infinity()},100,0.95);}, "infinite observation");
        rejects([&]{(void)bootstrap_mean_interval({1,std::numeric_limits<double>::quiet_NaN()},100,0.95);}, "NaN observation");
        std::cout << passed << "/" << passed << " checks passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

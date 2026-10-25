#include "quantiles.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) { std::cerr << "FAIL " << message << '\n'; std::exit(EXIT_FAILURE); }
}
void near(double a, double b, double tol, const char* message) {
    check(std::abs(a - b) <= tol, message);
}
template<typename Error,typename Fn>
void throws(Fn&& f) {
    bool caught = false;
    try { f(); } catch (const Error&) { caught = true; }
    check(caught, "expected exception");
}
}
int main() {
    using statistics10::quantile;
    using statistics10::interquartile_range;
    const std::vector<double> four{4,1,3,2};
    near(quantile(four, 0), 1, 1e-12, "minimum");
    near(quantile(four, 1), 4, 1e-12, "maximum");
    near(quantile(four, .25), 1.75, 1e-12, "q1");
    near(quantile(four, .5), 2.5, 1e-12, "median");
    near(quantile(four, .75), 3.25, 1e-12, "q3");
    near(interquartile_range(four), 1.5, 1e-12, "IQR");
    near(quantile({12}, .33), 12, 1e-12, "singleton");
    near(interquartile_range({7,7,7}), 0, 1e-12, "constant samples");
    near(quantile({-10,0,10},.5), 0, 1e-12, "negative numbers");
    throws<std::invalid_argument>([] { (void)quantile({}, .5); });
    throws<std::invalid_argument>([] { (void)quantile({1,2}, -0.01); });
    throws<std::invalid_argument>([] { (void)quantile({1,2}, 1.01); });
    throws<std::invalid_argument>([] {
        (void)quantile({1,2}, std::numeric_limits<double>::quiet_NaN());
    });
    throws<std::invalid_argument>([] {
        (void)quantile({1,std::numeric_limits<double>::infinity()}, .5);
    });
    for (int i = 0; i != 5000; ++i) {
        const double probability = static_cast<double>(i) / 4999.0;
        near(quantile({100,0}, probability), 100.0 * probability, 1e-12,
             "two-point interpolation");
    }
    std::cout << "PASS " << checks << " checks\n";
}

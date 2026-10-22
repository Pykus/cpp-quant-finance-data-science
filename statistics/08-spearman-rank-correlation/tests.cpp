#include "spearman.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int assertions = 0;
void check(bool condition, const std::string& description) {
    ++assertions;
    if (!condition) {
        throw std::runtime_error("FAIL: " + description);
    }
}
void near(double actual, double expected, double tolerance = 1e-12) {
    check(std::abs(actual - expected) <= tolerance, "unexpected correlation");
}
template <typename Error, typename Callable>
void expects(Callable&& fn) {
    bool caught = false;
    try {
        fn();
    } catch (const Error&) {
        caught = true;
    }
    check(caught, "expected error was not raised");
}
}  // namespace

int main() {
    using statistics::midranks;
    using statistics::spearman;
    try {
        near(spearman({1, 2, 3, 4}, {10, 20, 30, 40}).rho, 1.0);
        near(spearman({1, 2, 3, 4}, {40, 30, 20, 10}).rho, -1.0);
        near(spearman({1, 2, 3}, {3, 1, 2}).rho, -0.5);
        near(spearman({1, 1, 2, 3}, {1, 2, 2, 3}).rho, 5.0 / 6.0);
        near(spearman({-3, -2, -1}, {3, 2, 1}).rho, -1.0);
        near(spearman({1, 2}, {3, 4}).rho, 1.0);
        near(spearman({1, 2}, {4, 3}).rho, -1.0);

        const auto tied = midranks({1, 1, 2, 3});
        check(tied.ranks == std::vector<long double>({1.5L, 1.5L, 3.0L, 4.0L}),
              "midrank values");
        check(tied.tied_observations == 2, "tie count");
        const auto tied_more = midranks({4, 2, 4, 2, 2});
        check(tied_more.ranks == std::vector<long double>({4.5L, 2.0L, 4.5L, 2.0L, 2.0L}),
              "multiple tied groups");
        check(tied_more.tied_observations == 5, "multiple tied observations");
        check(midranks({}).ranks.empty(), "empty ranks are allowed");

        const auto metadata = spearman({1, 1, 2, 3}, {1, 2, 2, 3});
        check(metadata.pairs == 4, "pair count");
        check(metadata.tied_x == 2, "tied x count");
        check(metadata.tied_y == 2, "tied y count");

        expects<std::invalid_argument>([] { (void)spearman({}, {}); });
        expects<std::invalid_argument>([] { (void)spearman({1}, {1}); });
        expects<std::invalid_argument>([] { (void)spearman({1, 2}, {1}); });
        expects<std::invalid_argument>([] {
            (void)spearman({1, std::numeric_limits<double>::quiet_NaN()}, {1, 2});
        });
        expects<std::invalid_argument>([] {
            (void)spearman({1, 2}, {1, std::numeric_limits<double>::infinity()});
        });
        expects<std::invalid_argument>([] {
            (void)midranks({-std::numeric_limits<double>::infinity()});
        });
        expects<std::domain_error>([] { (void)spearman({1, 1, 1}, {1, 2, 3}); });
        expects<std::domain_error>([] { (void)spearman({1, 2, 3}, {5, 5, 5}); });
        expects<std::domain_error>([] { (void)spearman({1, 1}, {2, 2}); });

        const auto equal = spearman({2, 1, 1, 3, 4}, {2, 1, 1, 3, 4});
        near(equal.rho, 1.0);
        const auto inverted = spearman({1, 2, 2, 3}, {7, 5, 5, 1});
        near(inverted.rho, -1.0);
        const auto offset = spearman({1, 2, 3, 4, 5}, {1000, 2000, 3000, 4000, 5000});
        near(offset.rho, 1.0);
        const auto reversed = spearman({5, 4, 3, 2, 1}, {1000, 2000, 3000, 4000, 5000});
        near(reversed.rho, -1.0);
        const auto zeroes = midranks({-0.0, +0.0, 2.0});
        check(zeroes.tied_observations == 2, "signed zero tie");
        check(zeroes.ranks[0] == 1.5L && zeroes.ranks[1] == 1.5L,
              "zero midranks");

        std::cout << "PASS " << assertions << " assertions\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}

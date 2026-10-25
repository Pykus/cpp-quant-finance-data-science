#include "running_stats.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
int checks = 0;
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
void near(double actual, double expected, double tolerance, const char* message) {
    check(std::abs(actual - expected) <= tolerance, message);
}
template <typename Error, typename F>
void throws(F&& fn) {
    bool caught = false;
    try { fn(); } catch (const Error&) { caught = true; }
    check(caught, "expected exception");
}
}

int main() {
    using statistics09::RunningStats;
    RunningStats empty;
    check(empty.count() == 0, "empty count");
    throws<std::domain_error>([&] { (void)empty.mean(); });
    throws<std::domain_error>([&] { (void)empty.population_variance(); });
    throws<std::domain_error>([&] { (void)empty.sample_variance(); });

    RunningStats small;
    for (const double x : {2., 4., 4., 4., 5., 5., 7., 9.}) small.add(x);
    check(small.count() == 8, "8 observations");
    near(small.mean(), 5.0, 1e-12, "known mean");
    near(small.population_variance(), 4.0, 1e-12, "known population variance");
    near(small.sample_variance(), 32.0 / 7.0, 1e-12, "known sample variance");

    RunningStats singleton;
    singleton.add(42.0);
    near(singleton.mean(), 42, 1e-12, "singleton mean");
    near(singleton.population_variance(), 0, 1e-12, "singleton population variance");
    throws<std::domain_error>([&] { (void)singleton.sample_variance(); });

    throws<std::invalid_argument>([&] {
        small.add(std::numeric_limits<double>::quiet_NaN());
    });
    throws<std::invalid_argument>([&] {
        small.add(std::numeric_limits<double>::infinity());
    });
    check(small.count() == 8, "failed add did not mutate state");

    RunningStats full;
    RunningStats left;
    RunningStats right;
    RunningStats offset;
    for (int i = 0; i < 10000; ++i) {
        const double value = static_cast<double>(i % 5);
        full.add(value);
        (i < 5000 ? left : right).add(value);
        offset.add(1e12 + value);
        check(full.count() == static_cast<std::size_t>(i + 1), "running count");
    }
    left.merge(right);
    near(full.mean(), 2.0, 1e-12, "streaming mean");
    near(full.population_variance(), 2.0, 1e-10, "streaming population variance");
    near(full.sample_variance(), 20000.0 / 9999.0, 1e-10, "streaming sample variance");
    near(left.mean(), full.mean(), 1e-10, "merged mean");
    near(left.sample_variance(), full.sample_variance(), 1e-10, "merged variance");
    near(offset.mean(), 1e12 + 2, 0.01, "large-offset mean");
    near(offset.population_variance(), 2.0, 0.02, "large-offset stable variance");

    RunningStats empty_merge;
    full.merge(empty_merge);
    near(full.mean(), 2.0, 1e-12, "merging empty batch");
    empty_merge.merge(full);
    near(empty_merge.mean(), full.mean(), 1e-12, "empty recipient merge");

    RunningStats constant;
    for (int i = 0; i != 5000; ++i) constant.add(3.25);
    near(constant.sample_variance(), 0.0, 1e-12, "constant stream");

    std::cout << "PASS " << checks << " checks\n";
}

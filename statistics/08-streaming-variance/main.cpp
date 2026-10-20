#include "running_moments.hpp"
#include <iomanip>
#include <iostream>

int main() {
    stats::RunningMoments morning;
    stats::RunningMoments afternoon;
    for (double x : {10.0, 12.0}) morning.add(x);
    for (double x : {14.0, 16.0}) afternoon.add(x);
    morning.merge(afternoon);
    std::cout << std::fixed << std::setprecision(3)
              << "requests=" << morning.count() << "\n"
              << "mean_ms=" << morning.mean().value() << "\n"
              << "sample_variance_ms2=" << morning.sample_variance().value() << "\n"
              << "sample_stddev_ms=" << morning.sample_stddev().value() << "\n"
              << "min_ms=" << morning.minimum().value() << "\n"
              << "max_ms=" << morning.maximum().value() << "\n";
}

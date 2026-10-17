#include "bootstrap.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Fictional call-center waiting times in minutes.
    const std::vector<double> waiting_minutes{3, 5, 4, 7, 6, 9};
    const auto average = stats::bootstrap_interval(
        waiting_minutes, stats::Statistic::mean);
    const auto typical = stats::bootstrap_interval(
        waiting_minutes, stats::Statistic::median);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Observed mean wait: " << average.point_estimate << " min\n";
    std::cout << "95% bootstrap percentile interval for mean: ["
              << average.lower << ", " << average.upper << "] min\n";
    std::cout << "Observed median wait: " << typical.point_estimate << " min\n";
    std::cout << "95% bootstrap percentile interval for median: ["
              << typical.lower << ", " << typical.upper << "] min\n";
    std::cout << "Resamples per statistic: " << average.replicates << "\n";
}

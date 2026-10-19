#include "robust.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Fictional temperature sensor readings in degrees Celsius.
    const std::vector<double> readings{9.8, 10.0, 10.1, 10.2, 9.9, 10.0, 35.0};
    const auto report = statistics::detect_outliers(readings);

    std::cout << std::fixed << std::setprecision(3)
              << "Median: " << report.median
              << "; MAD: " << report.median_absolute_deviation << '\n';
    for (std::size_t i = 0; i < readings.size(); ++i) {
        std::cout << "row=" << i + 1 << " value=" << readings[i] << " z=";
        if (report.modified_z_scores[i]) {
            std::cout << *report.modified_z_scores[i];
        } else {
            std::cout << "undefined";
        }
        std::cout << '\n';
    }
    for (std::size_t index : report.flagged_indices) {
        std::cout << "REVIEW row " << index + 1 << '\n';
    }
}

#include "spearman.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    // Fictional survey: satisfaction (1-5) and repeat-visit intention (1-5).
    const std::vector<double> satisfaction{1, 2, 2, 3, 4, 5, 5, 4};
    const std::vector<double> revisit{1, 1, 3, 2, 4, 4, 5, 5};
    const auto result = statistics::spearman(satisfaction, revisit);
    std::cout << "pairs=" << result.pairs << " rho=" << std::fixed
              << std::setprecision(6) << result.rho << " tied_x="
              << result.tied_x << " tied_y=" << result.tied_y << '\n';
}

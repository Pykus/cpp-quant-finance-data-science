#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

struct Line { double intercept; double slope; };

Line fit_line(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2)
        throw std::invalid_argument("at least two paired observations required");
    double sx=0, sy=0;
    for (std::size_t i=0; i<x.size(); ++i) {
        if (!std::isfinite(x[i]) || !std::isfinite(y[i]))
            throw std::invalid_argument("values must be finite");
        sx += x[i]; sy += y[i];
    }
    double mx=sx/x.size(), my=sy/y.size();
    double numerator=0, denominator=0;
    for (std::size_t i=0; i<x.size(); ++i) {
        numerator += (x[i]-mx)*(y[i]-my);
        denominator += (x[i]-mx)*(x[i]-mx);
    }
    if (denominator == 0) throw std::invalid_argument("x must vary");
    double slope=numerator/denominator;
    return {my-slope*mx,slope};
}

#ifndef REGRESSION_TEST
int main() {
    const auto line=fit_line({1,2,3,4},{3,5,7,9});
    std::cout << std::fixed << std::setprecision(2)
              << "intercept=" << line.intercept << " slope=" << line.slope << '\n'
              << "prediction for x=5: " << line.intercept+line.slope*5 << '\n';
}
#endif

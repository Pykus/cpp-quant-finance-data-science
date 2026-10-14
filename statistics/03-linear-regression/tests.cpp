#define REGRESSION_TEST
#include "main.cpp"
#include <cassert>
int main() {
    auto line=fit_line({1,2,3,4},{3,5,7,9});
    assert(std::abs(line.intercept-1)<1e-12);
    assert(std::abs(line.slope-2)<1e-12);
    line=fit_line({1,2,3},{5,5,5});
    assert(std::abs(line.slope)<1e-12);
    bool rejected=false;
    try { fit_line({1,1,1},{1,2,3}); }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected);
}

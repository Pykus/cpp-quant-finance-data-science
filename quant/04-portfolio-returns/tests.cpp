#define PORTFOLIO_TEST
#include "main.cpp"
#include <cassert>
int main() {
    assert(std::abs(portfolio_return({0.6,0.4},{0.1,-0.05})-0.04)<1e-12);
    assert(std::abs(portfolio_return({1.0},{0.0}))<1e-12);
    bool rejected=false;
    try { portfolio_return({0.4,0.4},{0.1,0.2}); }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected);
    rejected=false;
    try { portfolio_return({1.0},{INFINITY}); }
    catch (const std::invalid_argument&) { rejected=true; }
    assert(rejected);
}

#define STAT_TEST
#include "main.cpp"
#include <cassert>
int main() {
    auto s = summarize({2, 4, 4, 6, 9});
    assert(std::abs(s.mean - 5.0) < 1e-12);
    assert(std::abs(s.median - 4.0) < 1e-12);
    assert(std::abs(s.population_variance - 5.6) < 1e-12);
    assert(summarize({3}).population_variance == 0);
    bool threw = false;
    try { summarize({}); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
}

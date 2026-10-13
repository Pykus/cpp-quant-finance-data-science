#define BOND_TEST
#include "main.cpp"
#include <cassert>
int main() {
    assert(std::abs(bond_price(1000, 0, 0, 2) - 1000) < 1e-9);
    assert(std::abs(macaulay_duration(1000, 0, 0.1, 2) - 2) < 1e-9);
    assert(bond_price(1000, 0.05, 0.06, 3) < 1000);
    assert(macaulay_duration(1000, 0.05, 0.06, 3) < 3);
    bool thrown = false;
    try { bond_price(1000, 0.05, -1, 3); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
}

#define RAII_TEST
#include "main.cpp"
#include <cassert>
#include <cstdio>
int main() {
    const std::string path = "raii_test_input.txt";
    { std::ofstream out(path); assert(out); out << "alpha\nbeta\n"; }
    assert(count_lines(path) == 2);
    std::remove(path.c_str());
    bool threw = false;
    try { count_lines(path); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);
}

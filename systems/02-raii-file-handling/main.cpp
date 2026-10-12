#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
std::size_t count_lines(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("cannot open input file");
    std::size_t count = 0;
    std::string line;
    while (std::getline(file, line)) ++count;
    if (file.bad()) throw std::runtime_error("error reading file");
    return count;
}
#ifndef RAII_TEST
int main(int argc, char** argv) {
    if (argc != 2) { std::cerr << "Usage: line_count FILE\n"; return 2; }
    try { std::cout << "lines=" << count_lines(argv[1]) << '\n'; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
#endif

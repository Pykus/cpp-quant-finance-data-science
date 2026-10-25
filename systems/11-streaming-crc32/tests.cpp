#include "crc32.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace {
int checks = 0;
void check(bool condition, const char* label) {
    ++checks;
    if (!condition) { std::cerr << "FAIL " << label << '\n'; std::exit(EXIT_FAILURE); }
}
}
int main() {
    using systems11::Crc32;
    using systems11::crc32_text;
    check(crc32_text("") == 0x00000000u, "empty known vector");
    check(crc32_text("123456789") == 0xcbf43926u, "standard known vector");
    check(crc32_text("hello") == 0x3610a686u, "hello known vector");
    check(crc32_text("The quick brown fox jumps over the lazy dog") == 0x414fa339u,
          "sentence known vector");

    Crc32 stream;
    stream.update_text("1234");
    stream.update_text("56789");
    check(stream.value() == 0xcbf43926u, "incremental split matches");
    stream.reset();
    check(stream.value() == 0, "reset");

    const std::array<std::byte, 3> raw{
        std::byte{0x41}, std::byte{0x42}, std::byte{0x43}
    };
    Crc32 binary;
    binary.update(raw);
    check(binary.value() == crc32_text("ABC"), "binary and text agree");

    // Exercise different split points without an external testing framework.
    for (std::size_t len = 0; len <= 10000; ++len) {
        std::string input(len, 'x');
        const auto expected = crc32_text(input);
        Crc32 incremental;
        const std::size_t split = len / 3;
        incremental.update_text(std::string_view{input}.substr(0, split));
        incremental.update_text(std::string_view{input}.substr(split));
        check(incremental.value() == expected, "chunked equals one-shot");
    }

    std::cout << "PASS " << checks << " checks\n";
}

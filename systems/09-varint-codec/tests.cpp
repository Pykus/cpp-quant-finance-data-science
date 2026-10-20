#include "varint.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
namespace {
std::uint64_t checks = 0;
void check(bool ok, const char* reason) {
    ++checks;
    if (!ok) throw std::runtime_error(reason);
}
using systems::DecodeStatus;
void round_trip(std::uint64_t value) {
    std::array<std::uint8_t, 12> buffer{};
    buffer.fill(0xeeU);
    const auto n = systems::encode(value, buffer);
    check(n == systems::encoded_size(value), "encoded length matches");
    check(n >= 1 && n <= 10, "bounded encoded length");
    check(buffer[n] == 0xeeU, "tail of output unchanged");
    const auto result = systems::decode(
        std::span<const std::uint8_t>(buffer.data(), n));
    check(result.status == DecodeStatus::complete, "roundtrip complete");
    check(result.value == value, "roundtrip value");
    check(result.bytes_consumed == n, "consumed exact length");
}
void fixed_values() {
    const std::array<std::uint64_t, 14> values{
        0ULL, 1ULL, 127ULL, 128ULL, 129ULL, 255ULL, 256ULL,
        16383ULL, 16384ULL, 300ULL, 15000ULL, 4294967295ULL,
        9223372036854775808ULL,
        std::numeric_limits<std::uint64_t>::max()};
    for (const std::uint64_t value : values) {
        round_trip(value);
    }
    std::array<std::uint8_t, 10> b{};
    auto n = systems::encode(300, b);
    check(n == 2, "300 takes two bytes");
    check(b[0] == 0xacU && b[1] == 0x02U, "known 300 encoding");
    n = systems::encode(std::numeric_limits<std::uint64_t>::max(), b);
    check(n == 10, "maximum takes ten bytes");
    check(b[9] == 1, "topmost bit encoding");
}
void truncation_and_streaming() {
    const std::array<std::uint8_t, 1> truncated{0xacU};
    const auto pending = systems::decode(truncated);
    check(pending.status == DecodeStatus::need_more, "truncated input pending");
    check(pending.bytes_consumed == 1, "one byte consumed");
    check(systems::decode(std::span<const std::uint8_t>{}).status
          == DecodeStatus::need_more, "empty input pending");
    systems::Decoder decoder;
    check(decoder.feed(0xacU) == DecodeStatus::need_more, "stream byte 1");
    bool threw = false;
    try { (void)decoder.value(); }
    catch (const std::logic_error&) { threw = true; }
    check(threw, "cannot read unfinished varint");
    check(decoder.feed(0x02U) == DecodeStatus::complete, "stream byte 2");
    check(decoder.value() == 300, "stream value");
    check(decoder.feed(0x00U) == DecodeStatus::invalid,
          "feeding after completion requires reset");
    decoder.reset();
    check(decoder.feed(0x01U) == DecodeStatus::complete, "reset can decode next");
    check(decoder.value() == 1, "next value after reset");
    const std::array<std::uint8_t, 3> combined{0xac, 0x02, 0x01};
    const auto first = systems::decode(combined);
    check(first.bytes_consumed == 2 && first.value == 300,
          "decode stops before second value");
}
void malformed_sequences() {
    const std::array<std::uint8_t, 2> overlong_zero{0x80, 0x00};
    const std::array<std::uint8_t, 2> overlong_one{0x81, 0x00};
    const std::array<std::uint8_t, 10> overflowing{
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02};
    const std::array<std::uint8_t, 10> never_ending{
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};
    const std::array<std::uint8_t, 10> long_zero{
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00};
    for (const auto seq : {
             std::span<const std::uint8_t>(overlong_zero),
             std::span<const std::uint8_t>(overlong_one),
             std::span<const std::uint8_t>(overflowing),
             std::span<const std::uint8_t>(never_ending),
             std::span<const std::uint8_t>(long_zero)}) {
        check(systems::decode(seq).status == DecodeStatus::invalid,
              "noncanonical or overflowing varint rejected");
    }
}
void insufficient_capacity() {
    std::array<std::uint8_t, 1> too_small{0x55};
    bool threw = false;
    try { (void)systems::encode(300, too_small); }
    catch (const std::length_error&) { threw = true; }
    check(threw, "short output buffer rejected");
    check(too_small[0] == 0x55, "failed encode does not modify output");
}
void deterministic_stress() {
    std::uint64_t state = 0xA5A5BEEF12345678ULL;
    for (int i = 0; i < 100000; ++i) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        round_trip(state);
    }
}
}
int main() {
    try {
        fixed_values();
        truncation_and_streaming();
        malformed_sequences();
        insufficient_capacity();
        deterministic_stress();
    } catch (const std::exception& e) {
        std::cerr << "FAIL at check " << checks << ": " << e.what() << '\n';
        return 1;
    }
    std::cout << "PASS: " << checks << " varint codec checks\n";
}

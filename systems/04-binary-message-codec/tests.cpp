#include "codec.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <variant>

namespace {
int checks = 0;
void check(bool success, const char* description) {
    ++checks;
    if (!success) throw std::runtime_error(description);
}
void add_roundtrip() {
    const wire::Add original{0x0102030405060708ULL, 0x11223344u, 0x55667788u};
    const auto packet = wire::encode(original);
    check(packet.has_value(), "add encoded");
    check(packet->size == 20, "add has 20 bytes");
    constexpr auto expected = std::array<std::uint8_t, 20>{
        1, 1, 0, 16, 1, 2, 3, 4, 5, 6, 7, 8,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };
    for (std::size_t i = 0; i < expected.size(); ++i) {
        check(std::to_integer<std::uint8_t>(packet->bytes[i]) == expected[i],
              "big-endian golden bytes");
    }
    const auto decoded = wire::decode(packet->view());
    check(static_cast<bool>(decoded), "add decoded");
    check(decoded.error == wire::Error::none, "add no error");
    check(std::get<wire::Add>(*decoded.message) == original, "add roundtrip values");
}
void cancel_roundtrip() {
    const wire::Cancel original{0x1122334455667788ULL};
    const auto packet = wire::encode(original);
    check(packet.has_value(), "cancel encoded");
    check(packet->size == 12, "cancel has 12 bytes");
    const auto decoded = wire::decode(packet->view());
    check(static_cast<bool>(decoded), "cancel decoded");
    check(std::get<wire::Cancel>(*decoded.message) == original, "cancel roundtrip values");
    check(!wire::encode(wire::Cancel{0}).has_value(), "zero cancel ID rejected");
}
void invalid_frames() {
    check(!wire::encode(wire::Add{0, 100, 2}), "zero add ID rejected");
    check(!wire::encode(wire::Add{7, 0, 2}), "zero price rejected");
    check(!wire::encode(wire::Add{7, 100, 0}), "zero quantity rejected");
    const auto add = *wire::encode(wire::Add{7, 100, 2});
    for (std::size_t n = 0; n < add.size; ++n) {
        const auto decoded = wire::decode(add.view().first(n));
        check(decoded.error == wire::Error::truncated, "truncated add prefix");
        check(!decoded.message.has_value(), "truncated add has no message");
    }
    const auto cancel = *wire::encode(wire::Cancel{7});
    for (std::size_t n = 0; n < cancel.size; ++n) {
        check(wire::decode(cancel.view().first(n)).error == wire::Error::truncated,
              "truncated cancel prefix");
    }
    auto changed = add;
    changed.bytes[0] = std::byte{2};
    check(wire::decode(changed.view()).error == wire::Error::unsupported_version,
          "unsupported version");
    changed = add;
    changed.bytes[1] = std::byte{99};
    check(wire::decode(changed.view()).error == wire::Error::unknown_kind,
          "unknown message kind");
    changed = add;
    changed.bytes[3] = std::byte{17};
    check(wire::decode(changed.view()).error == wire::Error::invalid_length,
          "declared payload length mismatch");
    std::array<std::byte, 21> too_long{};
    std::copy(add.bytes.begin(), add.bytes.end(), too_long.begin());
    check(wire::decode(too_long).error == wire::Error::trailing_bytes,
          "trailing bytes rejected");
    changed = add;
    std::fill(changed.bytes.begin() + 4, changed.bytes.begin() + 12, std::byte{0});
    check(wire::decode(changed.view()).error == wire::Error::invalid_field,
          "zero wire ID rejected");
    changed = add;
    std::fill(changed.bytes.begin() + 12, changed.bytes.begin() + 16, std::byte{0});
    check(wire::decode(changed.view()).error == wire::Error::invalid_field,
          "zero wire price rejected");
    changed = add;
    std::fill(changed.bytes.begin() + 16, changed.bytes.begin() + 20, std::byte{0});
    check(wire::decode(changed.view()).error == wire::Error::invalid_field,
          "zero wire quantity rejected");
    changed = cancel;
    std::fill(changed.bytes.begin() + 4, changed.bytes.begin() + 12, std::byte{0});
    check(wire::decode(changed.view()).error == wire::Error::invalid_field,
          "zero cancel ID decoded rejected");
}
} // namespace
int main() {
    try {
        add_roundtrip();
        cancel_roundtrip();
        invalid_frames();
        std::cout << "PASS: " << checks << " checks\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << " after " << checks << " checks\n";
        return 1;
    }
}

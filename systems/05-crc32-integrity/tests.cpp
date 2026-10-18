#include "crc32.hpp"
#include "../04-binary-message-codec/codec.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>
#include <variant>

namespace {
int checks = 0;

void require(bool condition, const char* description) {
    ++checks;
    if (!condition) {
        throw std::runtime_error(description);
    }
}

[[nodiscard]] std::span<const std::byte> as_bytes(std::string_view text) {
    return std::as_bytes(std::span{text.data(), text.size()});
}

void standard_reference_vectors() {
    require(integrity::crc32({}) == 0x00000000u, "empty CRC-32 is zero");
    require(integrity::crc32(as_bytes("123456789")) == 0xCBF43926u,
            "ISO-HDLC reference vector");
    require(integrity::crc32(as_bytes("a")) == 0xE8B7BE43u,
            "single letter a reference vector");
    require(integrity::crc32(as_bytes("A")) == 0xD3D99E8Bu,
            "single letter A reference vector");
    constexpr std::array<std::byte, 1> zero{std::byte{0}};
    require(integrity::crc32(zero) == 0xD202EF8Du,
            "single zero byte vector");
    std::array<std::byte, 256> sequence{};
    for (std::size_t i = 0; i < sequence.size(); ++i) {
        sequence[i] = static_cast<std::byte>(i);
    }
    require(integrity::crc32(sequence) == 0x29058C73u,
            "all byte values reference vector");
}

void streaming_is_equivalent_to_single_pass() {
    const auto bytes = as_bytes("123456789");
    const auto expected = integrity::crc32(bytes);
    for (std::size_t split = 0; split <= bytes.size(); ++split) {
        integrity::Crc32 stream;
        stream.update(bytes.first(split));
        stream.update(bytes.subspan(split));
        require(stream.value() == expected, "two chunks match one pass");
        require(stream.value() == expected, "reading value is repeatable");
    }
    integrity::Crc32 stream;
    for (const auto byte : bytes) {
        const auto one = std::span<const std::byte>{&byte, 1};
        stream.update(one);
    }
    require(stream.value() == expected, "one byte at a time matches");
    stream.reset();
    require(stream.value() == 0u, "reset produces empty checksum");
    stream.update(bytes);
    require(stream.value() == expected, "reset supports reuse");
    integrity::Crc32 prefix;
    prefix.update(as_bytes("123"));
    const auto checkpoint = prefix.value();
    require(checkpoint == integrity::crc32(as_bytes("123")),
            "intermediate checksum agrees");
    prefix.update(as_bytes("456789"));
    require(prefix.value() == expected,
            "reading intermediate value does not finalize stream");
}

void bounded_trailer_and_validation() {
    require(!integrity::append_crc({}).has_value(), "empty payload rejected");
    std::array<std::byte, 21> oversized{};
    require(!integrity::append_crc(oversized).has_value(),
            "payload larger than maximum rejected");

    const auto checked = integrity::append_crc(as_bytes("123456789"));
    require(checked.has_value(), "reference packet created");
    require(checked->size == 13, "nine bytes plus four trailer bytes");
    const std::array<std::uint8_t, 4> expected_trailer{0xCB, 0xF4, 0x39, 0x26};
    for (std::size_t i = 0; i < expected_trailer.size(); ++i) {
        require(std::to_integer<std::uint8_t>(checked->bytes[9 + i]) ==
                    expected_trailer[i], "checksum trailer is network order");
    }
    require(integrity::verify_crc(checked->view()), "valid packet accepted");
    for (std::size_t length = 0; length <= 4; ++length) {
        require(!integrity::verify_crc(checked->view().first(length)),
                "empty payload or incomplete CRC rejected");
    }
    auto altered_data = *checked;
    altered_data.bytes[2] ^= std::byte{1};
    require(!integrity::verify_crc(altered_data.view()),
            "one altered data bit detected");
    auto altered_trailer = *checked;
    altered_trailer.bytes[12] ^= std::byte{1};
    require(!integrity::verify_crc(altered_trailer.view()),
            "one altered checksum bit detected");
    require(!integrity::verify_crc(checked->view().first(12)),
            "missing final checksum byte rejected");
    std::array<std::byte, 25> too_long{};
    require(!integrity::verify_crc(too_long), "oversized received frame rejected");
    std::array<std::byte, 5> smallest_payload{};
    smallest_payload[0] = std::byte{0x7A};
    const auto single = integrity::append_crc(
        std::span<const std::byte>{smallest_payload.data(), 1});
    require(single && single->size == 5, "one-byte payload supported");
    require(integrity::verify_crc(single->view()),
            "one-byte payload roundtrip");

    for (std::size_t length = 1; length <= 20; ++length) {
        std::array<std::byte, 20> bytes{};
        for (std::size_t i = 0; i < length; ++i) {
            bytes[i] = static_cast<std::byte>((i * 31u + length) & 0xFFu);
        }
        const auto framed = integrity::append_crc(
            std::span<const std::byte>{bytes.data(), length});
        require(framed.has_value(), "allowed length framed");
        require(framed->size == length + 4, "framed length correct");
        require(integrity::verify_crc(framed->view()), "framed data validates");
    }
}

void integrate_with_previous_wire_codec() {
    const auto add = wire::encode(wire::Add{7, 10123, 250});
    const auto cancel = wire::encode(wire::Cancel{7});
    require(add.has_value() && cancel.has_value(), "sample events encoded");
    for (const auto source : {*add, *cancel}) {
        const auto protected_frame = integrity::append_crc(source.view());
        require(protected_frame.has_value(), "event protected");
        require(protected_frame->size == source.size + 4,
                "protected event length");
        require(integrity::verify_crc(protected_frame->view()),
                "untouched event passes CRC");
        const auto parsed = wire::decode(
            protected_frame->view().first(source.size));
        require(parsed.message.has_value(), "inner protocol still parses");
        auto modified = *protected_frame;
        modified.bytes[5] ^= std::byte{1};
        require(!integrity::verify_crc(modified.view()),
                "altered event fails CRC before parsing");
    }
    // A correct CRC is not proof that an *inner* packet is well-formed.
    std::array<std::byte, 12> malformed{};
    const auto wrapped = integrity::append_crc(malformed);
    require(wrapped.has_value() && integrity::verify_crc(wrapped->view()),
            "CRC does not validate inner message semantics");
    require(!wire::decode(wrapped->view().first(12)).message.has_value(),
            "inner decoder still rejects malformed message");
}
}  // namespace

int main() {
    try {
        standard_reference_vectors();
        streaming_is_equivalent_to_single_pass();
        bounded_trailer_and_validation();
        integrate_with_previous_wire_codec();
        std::cout << "PASS: " << checks << " checks\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << " after " << checks << " checks\n";
        return 1;
    }
    return 0;
}

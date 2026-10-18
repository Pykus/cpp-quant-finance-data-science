#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace integrity {

// CRC-32/ISO-HDLC: reflected polynomial, all-one initial and final XOR values.
inline constexpr std::uint32_t kReflectedPolynomial = 0xEDB88320u;
inline constexpr std::size_t kMaximumPayloadBytes = 20;
inline constexpr std::size_t kTrailerBytes = 4;

class Crc32 {
public:
    // No allocations; process input a byte at a time, in the supplied order.
    void update(std::span<const std::byte> input) noexcept {
        for (const auto byte : input) {
            remainder_ ^= std::to_integer<std::uint8_t>(byte);
            for (unsigned bit = 0; bit < 8; ++bit) {
                const bool low_bit = (remainder_ & 1u) != 0u;
                remainder_ >>= 1u;
                if (low_bit) {
                    remainder_ ^= kReflectedPolynomial;
                }
            }
        }
    }

    // Reporting the current checksum does not prevent further updates.
    [[nodiscard]] std::uint32_t value() const noexcept {
        return remainder_ ^ 0xFFFFFFFFu;
    }

    void reset() noexcept { remainder_ = 0xFFFFFFFFu; }

private:
    std::uint32_t remainder_ = 0xFFFFFFFFu;
};

[[nodiscard]] inline std::uint32_t crc32(
    std::span<const std::byte> input) noexcept {
    Crc32 calculator;
    calculator.update(input);
    return calculator.value();
}

struct ProtectedPacket {
    std::array<std::byte, kMaximumPayloadBytes + kTrailerBytes> bytes{};
    std::size_t size = 0;

    [[nodiscard]] std::span<const std::byte> view() const noexcept {
        return {bytes.data(), size};
    }
};

// Store four checksum bytes in network byte order (highest byte first).
// This adds an integrity trailer; it does not validate the inner protocol.
[[nodiscard]] inline std::optional<ProtectedPacket> append_crc(
    std::span<const std::byte> payload) noexcept {
    if (payload.empty() || payload.size() > kMaximumPayloadBytes) {
        return std::nullopt;
    }
    ProtectedPacket packet;
    std::copy(payload.begin(), payload.end(), packet.bytes.begin());
    const auto checksum = crc32(payload);
    for (std::size_t i = 0; i < kTrailerBytes; ++i) {
        const unsigned shift = static_cast<unsigned>(8 * (3 - i));
        packet.bytes[payload.size() + i] =
            static_cast<std::byte>((checksum >> shift) & 0xFFu);
    }
    packet.size = payload.size() + kTrailerBytes;
    return packet;
}

// A CRC detects accidental changes, NOT deliberate tampering.
// The caller must still validate the inner packet's length and fields.
[[nodiscard]] inline bool verify_crc(
    std::span<const std::byte> packet) noexcept {
    if (packet.size() <= kTrailerBytes ||
        packet.size() > kMaximumPayloadBytes + kTrailerBytes) {
        return false;
    }
    const std::size_t payload_size = packet.size() - kTrailerBytes;
    const auto expected = crc32(packet.first(payload_size));
    std::uint32_t supplied = 0;
    for (std::size_t i = 0; i < kTrailerBytes; ++i) {
        supplied = (supplied << 8u) |
            std::to_integer<std::uint8_t>(packet[payload_size + i]);
    }
    return supplied == expected;
}

}  // namespace integrity

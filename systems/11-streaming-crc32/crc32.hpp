#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace systems11 {

// CRC-32/ISO-HDLC (IEEE): reflected polynomial 0xEDB88320,
// initial state 0xFFFFFFFF, final XOR 0xFFFFFFFF.
class Crc32 {
public:
    void reset() noexcept { state_ = 0xffffffffu; }

    void update(std::span<const std::byte> bytes) noexcept {
        for (const auto item : bytes) {
            step(std::to_integer<std::uint8_t>(item));
        }
    }

    void update_text(std::string_view text) noexcept {
        for (unsigned char character : text) {
            step(static_cast<std::uint8_t>(character));
        }
    }

    [[nodiscard]] std::uint32_t value() const noexcept {
        return state_ ^ 0xffffffffu;
    }

private:
    void step(std::uint8_t octet) noexcept {
        state_ ^= octet;
        for (int bit = 0; bit < 8; ++bit) {
            const std::uint32_t mask = 0u - (state_ & 1u);
            state_ = (state_ >> 1) ^ (0xedb88320u & mask);
        }
    }

    std::uint32_t state_ = 0xffffffffu;
};

[[nodiscard]] inline std::uint32_t crc32_text(std::string_view text) noexcept {
    Crc32 checksum;
    checksum.update_text(text);
    return checksum.value();
}

}  // namespace systems11

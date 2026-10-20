#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace systems {

enum class DecodeStatus { need_more, complete, invalid };

struct DecodeResult {
    DecodeStatus status;
    std::size_t bytes_consumed;
    std::uint64_t value;
};

[[nodiscard]] constexpr std::size_t encoded_size(std::uint64_t value) noexcept {
    std::size_t bytes = 1;
    while (value >= 0x80U) {
        value >>= 7U;
        ++bytes;
    }
    return bytes;
}

// All-or-nothing encode: checks capacity before touching the output buffer.
[[nodiscard]] inline std::size_t encode(std::uint64_t value,
                                        std::span<std::uint8_t> output) {
    const std::size_t required = encoded_size(value);
    if (output.size() < required) {
        throw std::length_error("Output buffer too small for unsigned varint");
    }
    std::size_t i = 0;
    while (value >= 0x80U) {
        output[i++] = static_cast<std::uint8_t>((value & 0x7fU) | 0x80U);
        value >>= 7U;
    }
    output[i++] = static_cast<std::uint8_t>(value);
    return i;
}

// Explicit incremental decoder for fragmented network inputs.
class Decoder {
public:
    [[nodiscard]] DecodeStatus feed(std::uint8_t byte) noexcept {
        if (status_ != DecodeStatus::need_more) {
            status_ = DecodeStatus::invalid; // Explicit reset required for next value.
            return status_;
        }
        // At position 10 only the lowest data bit may be set.
        if (consumed_ == 9 && (byte & 0xfeU) != 0) {
            ++consumed_;
            status_ = DecodeStatus::invalid;
            return status_;
        }
        accumulated_ |= (static_cast<std::uint64_t>(byte & 0x7fU)
                         << (7U * consumed_));
        ++consumed_;
        if ((byte & 0x80U) != 0) {
            if (consumed_ == 10) status_ = DecodeStatus::invalid;
        } else if (consumed_ > 1 && (byte & 0x7fU) == 0) {
            status_ = DecodeStatus::invalid; // Noncanonical overlong representation.
        } else {
            status_ = DecodeStatus::complete;
        }
        return status_;
    }

    [[nodiscard]] DecodeStatus status() const noexcept { return status_; }
    [[nodiscard]] std::size_t bytes_consumed() const noexcept { return consumed_; }
    [[nodiscard]] std::uint64_t value() const {
        if (status_ != DecodeStatus::complete) {
            throw std::logic_error("Varint has not been completely decoded");
        }
        return accumulated_;
    }
    void reset() noexcept {
        status_ = DecodeStatus::need_more;
        accumulated_ = 0;
        consumed_ = 0;
    }

private:
    DecodeStatus status_{DecodeStatus::need_more};
    std::uint64_t accumulated_{0};
    std::size_t consumed_{0};
};

[[nodiscard]] inline DecodeResult decode(
    std::span<const std::uint8_t> input) noexcept {
    Decoder decoder;
    for (const auto byte : input) {
        const auto status = decoder.feed(byte);
        if (status != DecodeStatus::need_more) {
            return {status, decoder.bytes_consumed(),
                    status == DecodeStatus::complete ? decoder.value() : 0U};
        }
    }
    return {DecodeStatus::need_more, decoder.bytes_consumed(), 0U};
}

} // namespace systems

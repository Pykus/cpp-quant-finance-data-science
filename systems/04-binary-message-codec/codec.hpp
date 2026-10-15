#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <variant>

namespace wire {
inline constexpr std::size_t kHeader = 4;
inline constexpr std::size_t kAddSize = 20;
inline constexpr std::size_t kCancelSize = 12;

struct Add {
    std::uint64_t id;
    std::uint32_t price_ticks;
    std::uint32_t quantity;
    bool operator==(const Add&) const = default;
};
struct Cancel {
    std::uint64_t id;
    bool operator==(const Cancel&) const = default;
};
using Message = std::variant<Add, Cancel>;
enum class Error { none, truncated, unsupported_version, unknown_kind,
                   invalid_length, trailing_bytes, invalid_field };
struct Result {
    std::optional<Message> message;
    Error error = Error::none;
    [[nodiscard]] explicit operator bool() const noexcept { return message.has_value(); }
};
struct Frame {
    std::array<std::byte, kAddSize> bytes{};
    std::size_t size{};
    [[nodiscard]] std::span<const std::byte> view() const noexcept {
        return {bytes.data(), size};
    }
};
namespace detail {
inline void write_big_endian(Frame& frame, std::size_t position,
                             std::uint64_t value, std::size_t count) noexcept {
    for (std::size_t i = 0; i < count; ++i) {
        frame.bytes[position + i] = static_cast<std::byte>(
            (value >> (8 * (count - 1 - i))) & 0xffu);
    }
}
[[nodiscard]] inline std::uint64_t read_big_endian(
    std::span<const std::byte> bytes, std::size_t position, std::size_t count) noexcept {
    std::uint64_t result = 0;
    for (std::size_t i = 0; i < count; ++i) {
        result = (result << 8) | std::to_integer<std::uint8_t>(bytes[position + i]);
    }
    return result;
}
[[nodiscard]] inline Frame header(std::uint8_t kind, std::size_t payload) noexcept {
    Frame frame;
    frame.size = kHeader + payload;
    frame.bytes[0] = std::byte{1};
    frame.bytes[1] = static_cast<std::byte>(kind);
    write_big_endian(frame, 2, payload, 2);
    return frame;
}
} // namespace detail

[[nodiscard]] inline std::optional<Frame> encode(Add add) noexcept {
    if (add.id == 0 || add.price_ticks == 0 || add.quantity == 0) {
        return std::nullopt;
    }
    auto frame = detail::header(1, 16);
    detail::write_big_endian(frame, 4, add.id, 8);
    detail::write_big_endian(frame, 12, add.price_ticks, 4);
    detail::write_big_endian(frame, 16, add.quantity, 4);
    return frame;
}
[[nodiscard]] inline std::optional<Frame> encode(Cancel cancel) noexcept {
    if (cancel.id == 0) return std::nullopt;
    auto frame = detail::header(2, 8);
    detail::write_big_endian(frame, 4, cancel.id, 8);
    return frame;
}

// One complete packet, not a TCP stream. Never read beyond the supplied span.
[[nodiscard]] inline Result decode(std::span<const std::byte> bytes) noexcept {
    const auto fail = [](Error e) -> Result { return {std::nullopt, e}; };
    if (bytes.size() < kHeader) return fail(Error::truncated);
    if (bytes[0] != std::byte{1}) return fail(Error::unsupported_version);
    const auto kind = std::to_integer<std::uint8_t>(bytes[1]);
    std::size_t payload = 0;
    if (kind == 1) payload = 16;
    else if (kind == 2) payload = 8;
    else return fail(Error::unknown_kind);
    if (detail::read_big_endian(bytes, 2, 2) != payload) {
        return fail(Error::invalid_length);
    }
    if (bytes.size() < kHeader + payload) return fail(Error::truncated);
    if (bytes.size() > kHeader + payload) return fail(Error::trailing_bytes);
    const auto id = detail::read_big_endian(bytes, 4, 8);
    if (id == 0) return fail(Error::invalid_field);
    if (kind == 2) return {Message{Cancel{id}}, Error::none};
    const auto price = static_cast<std::uint32_t>(detail::read_big_endian(bytes, 12, 4));
    const auto quantity = static_cast<std::uint32_t>(detail::read_big_endian(bytes, 16, 4));
    if (price == 0 || quantity == 0) return fail(Error::invalid_field);
    return {Message{Add{id, price, quantity}}, Error::none};
}
} // namespace wire

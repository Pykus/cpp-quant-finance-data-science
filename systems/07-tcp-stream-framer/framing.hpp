#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <utility>

namespace stream_framing {

// Wire format: [two-byte unsigned big-endian payload length][payload bytes].
// The length never includes the two-byte header. Zero-length frames are invalid.
template <std::size_t MaxPayload>
struct EncodedFrame {
    static_assert(MaxPayload >= 1 && MaxPayload <= 65535);

    std::array<std::byte, MaxPayload + 2> bytes{};
    std::size_t used{};

    [[nodiscard]] std::span<const std::byte> view() const noexcept {
        return {bytes.data(), used};
    }
};

template <std::size_t MaxPayload>
[[nodiscard]] std::optional<EncodedFrame<MaxPayload>>
encode_frame(std::span<const std::byte> payload) noexcept {
    static_assert(MaxPayload >= 1 && MaxPayload <= 65535);
    if (payload.empty() || payload.size() > MaxPayload) {
        return std::nullopt;
    }

    EncodedFrame<MaxPayload> result{};
    const auto length = payload.size();
    result.bytes[0] = std::byte{static_cast<unsigned char>((length >> 8U) & 0xFFU)};
    result.bytes[1] = std::byte{static_cast<unsigned char>(length & 0xFFU)};
    std::copy(payload.begin(), payload.end(), result.bytes.begin() + 2);
    result.used = length + 2;
    return result;
}

enum class FeedStatus {
    ok,
    invalid_length
};

struct FeedResult {
    FeedStatus status{};
    std::size_t bytes_consumed{};
    std::size_t frames_emitted{};
};

// Stateful decoder for consecutive frames carried by an arbitrary byte stream.
// It never allocates and stores at most MaxPayload bytes plus two header bytes.
// The handler receives a borrowed view: copy it before the next frame is parsed.
// Feed and reset must not be called concurrently or re-entrantly on one decoder.
template <std::size_t MaxPayload = 64>
class StreamDecoder {
    static_assert(MaxPayload >= 1 && MaxPayload <= 65535);

public:
    [[nodiscard]] bool failed() const noexcept { return failed_; }
    [[nodiscard]] std::size_t buffered_bytes() const noexcept {
        return prefix_bytes_ + payload_bytes_;
    }

    void reset() noexcept {
        reset_frame();
        failed_ = false;
    }

    template <typename Handler>
    [[nodiscard]] FeedResult feed(std::span<const std::byte> chunk,
                                  Handler&& on_frame) {
        if (failed_) {
            return {FeedStatus::invalid_length, 0, 0};
        }

        std::size_t completed = 0;
        for (std::size_t i = 0; i < chunk.size(); ++i) {
            if (prefix_bytes_ < 2) {
                announced_length_ = static_cast<std::uint16_t>(
                    (static_cast<unsigned int>(announced_length_) << 8U)
                    | std::to_integer<unsigned int>(chunk[i]));
                ++prefix_bytes_;

                if (prefix_bytes_ == 2
                    && (announced_length_ == 0 || announced_length_ > MaxPayload)) {
                    failed_ = true;
                    return {FeedStatus::invalid_length, i + 1, completed};
                }
                continue;
            }

            payload_[payload_bytes_++] = chunk[i];
            if (payload_bytes_ == announced_length_) {
                const auto message = std::span<const std::byte>{
                    payload_.data(), payload_bytes_};

                // Make the decoder ready for the next frame before callback.
                // If a callback throws, its exception propagates to the caller.
                reset_frame();
                std::invoke(on_frame, message);
                ++completed;
            }
        }
        return {FeedStatus::ok, chunk.size(), completed};
    }

private:
    void reset_frame() noexcept {
        announced_length_ = 0;
        prefix_bytes_ = 0;
        payload_bytes_ = 0;
    }

    std::array<std::byte, MaxPayload> payload_{};
    std::uint16_t announced_length_{};
    std::size_t prefix_bytes_{};
    std::size_t payload_bytes_{};
    bool failed_{};
};

} // namespace stream_framing

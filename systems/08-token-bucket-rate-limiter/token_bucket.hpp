#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>

namespace systems {

// A bounded, single-threaded token bucket. Supply a monotonic clock explicitly
// so real code and deterministic tests use the same implementation.
class TokenBucket final {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    TokenBucket(std::uint32_t capacity,
                std::uint32_t refill_per_second,
                TimePoint start)
        : capacity_(capacity),
          refill_per_second_(refill_per_second),
          tokens_(capacity),
          last_refill_(start) {
        // Bounds also make the intermediate integer calculations provably safe.
        if (capacity == 0 || capacity > 1'000'000 ||
            refill_per_second == 0 || refill_per_second > 1'000'000) {
            throw std::invalid_argument("capacity and rate must be in [1,1000000]");
        }
    }

    [[nodiscard]] bool try_acquire(std::uint32_t amount,
                                   TimePoint now) noexcept {
        refill(now);
        if (amount > tokens_) {
            return false;
        }
        tokens_ -= amount;
        return true;
    }

    [[nodiscard]] std::uint32_t available(TimePoint now) noexcept {
        refill(now);
        return tokens_;
    }

    [[nodiscard]] std::uint32_t capacity() const noexcept {
        return capacity_;
    }

    [[nodiscard]] std::uint32_t refill_rate() const noexcept {
        return refill_per_second_;
    }

private:
    void refill(TimePoint now) noexcept {
        if (now <= last_refill_) {
            return;  // A backwards timestamp must not manufacture credits.
        }

        // Millisecond granularity is intentional; sub-ms time is retained in
        // last_refill_ until a whole millisecond has elapsed.
        const auto elapsed_signed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_refill_).count();
        if (elapsed_signed <= 0) {
            return;
        }
        const auto elapsed = static_cast<std::uint64_t>(elapsed_signed);
        last_refill_ += std::chrono::milliseconds(elapsed_signed);

        // If enough time has elapsed to fill even an empty bucket, saturate
        // before multiplying. This also avoids overflow for long idle spans.
        const std::uint64_t saturation_ms =
            (static_cast<std::uint64_t>(capacity_) * 1'000ULL +
             refill_per_second_ - 1ULL) / refill_per_second_;
        if (elapsed >= saturation_ms) {
            tokens_ = capacity_;
            remainder_ = 0;
            return;
        }

        // elapsed < saturation_ms <= 1e9 and rate <= 1e6.
        // Therefore accrued milli-tokens comfortably fit in uint64_t.
        const std::uint64_t accrued =
            elapsed * refill_per_second_ + remainder_;
        const std::uint64_t added = accrued / 1'000ULL;
        remainder_ = static_cast<std::uint32_t>(accrued % 1'000ULL);
        const std::uint64_t updated =
            static_cast<std::uint64_t>(tokens_) + added;
        tokens_ = static_cast<std::uint32_t>(
            std::min(updated, static_cast<std::uint64_t>(capacity_)));

        // Credit cannot accumulate beyond the burst capacity.
        if (tokens_ == capacity_) {
            remainder_ = 0;
        }
    }

    std::uint32_t capacity_;
    std::uint32_t refill_per_second_;
    std::uint32_t tokens_;
    std::uint32_t remainder_{0};  // Fractional token units: 0..999.
    TimePoint last_refill_;
};

}  // namespace systems

#pragma once

#include <cstdint>

namespace systems09 {

enum class EventKind {
    First,
    InOrder,
    Gap,
    LateOrDuplicate
};

struct Observation {
    EventKind kind;
    std::uint32_t missing;
    std::uint32_t next_expected;
};

// Sequence numbers are uint32_t values increasing modulo 2^32.
// This is a best-effort detector, not a full reorder buffer.
class SequenceTracker {
public:
    [[nodiscard]] Observation observe(std::uint32_t sequence) noexcept {
        if (!initialized_) {
            initialized_ = true;
            expected_ = sequence + std::uint32_t{1};  // defined unsigned wraparound
            return {EventKind::First, 0, expected_};
        }
        const std::uint32_t distance = sequence - expected_;
        if (distance == 0) {
            expected_ = sequence + std::uint32_t{1};
            return {EventKind::InOrder, 0, expected_};
        }
        // Interpreting modular differences >= 2^31 as "future" is ambiguous.
        // Reject those as late/duplicate rather than invent enormous gaps.
        if (distance < std::uint32_t{0x80000000u}) {
            expected_ = sequence + std::uint32_t{1};
            return {EventKind::Gap, distance, expected_};
        }
        return {EventKind::LateOrDuplicate, 0, expected_};
    }

    void reset() noexcept {
        initialized_ = false;
        expected_ = 0;
    }

    [[nodiscard]] bool initialized() const noexcept { return initialized_; }
    [[nodiscard]] std::uint32_t next_expected() const noexcept { return expected_; }

private:
    bool initialized_ = false;
    std::uint32_t expected_ = 0;
};

}  // namespace systems09

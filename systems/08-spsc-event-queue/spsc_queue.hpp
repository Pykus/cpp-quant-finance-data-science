#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <type_traits>

namespace systems {

// A bounded, single-producer/single-consumer queue. Exactly one thread may
// call try_push() and exactly one OTHER thread may call try_pop() at a time.
// Never call these operations from multiple producers or consumers.
template <typename T, std::size_t Capacity>
class SpscQueue final {
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two and at least two");
    static_assert(std::is_trivially_copyable_v<T>,
                  "Queue events must be trivially copyable");
    static_assert(std::is_default_constructible_v<T>,
                  "Queue events must be default constructible");
    static_assert(std::is_nothrow_copy_assignable_v<T>,
                  "Queue event assignments must not throw");

public:
    SpscQueue() = default;
    SpscQueue(const SpscQueue&) = delete;
    SpscQueue& operator=(const SpscQueue&) = delete;
    SpscQueue(SpscQueue&&) = delete;
    SpscQueue& operator=(SpscQueue&&) = delete;

    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    // Returns false without modifying the queue when full.
    [[nodiscard]] bool try_push(const T& value) noexcept {
        const auto head = head_.value.load(std::memory_order_relaxed);
        const auto tail = tail_.value.load(std::memory_order_acquire);
        if (head - tail == Capacity) {
            return false;
        }

        buffer_[head & (Capacity - 1)] = value;
        head_.value.store(head + 1, std::memory_order_release);
        return true;
    }

    // Returns false without modifying output when empty.
    [[nodiscard]] bool try_pop(T& output) noexcept {
        const auto tail = tail_.value.load(std::memory_order_relaxed);
        const auto head = head_.value.load(std::memory_order_acquire);
        if (tail == head) {
            return false;
        }

        output = buffer_[tail & (Capacity - 1)];
        tail_.value.store(tail + 1, std::memory_order_release);
        return true;
    }

private:
    // Separate the two frequently written counters onto different cache lines.
    // 64 bytes is a common cache line size, not a portable hardware guarantee.
    struct alignas(64) PaddedIndex {
        std::atomic<std::size_t> value{0};
    };

    PaddedIndex head_{};  // Written by the producer, read by the consumer.
    PaddedIndex tail_{};  // Written by the consumer, read by the producer.
    alignas(64) std::array<T, Capacity> buffer_{};
};

}  // namespace systems

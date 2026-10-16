#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace market_data {

// A bounded FIFO for exactly one producer and one consumer.
// The producer calls try_push; the consumer calls try_pop.
// Both threads must finish before queue destruction.
template <typename T, std::size_t Capacity>
class SpscEventQueue final {
    static_assert(Capacity > 0, "Capacity must be positive");
    static_assert(Capacity < std::numeric_limits<std::size_t>::max(),
                  "Capacity is too large");
    static_assert(std::is_trivially_copyable_v<T>,
                  "Use a trivially copyable message type");
    static_assert(std::is_default_constructible_v<T>,
                  "Message type must be default constructible");
    static_assert(std::is_nothrow_copy_assignable_v<T>,
                  "Message assignment must not throw");

    // An extra slot distinguishes full from empty without a shared counter.
    static constexpr std::size_t storage_capacity = Capacity + 1;

public:
    SpscEventQueue() = default;
    SpscEventQueue(const SpscEventQueue&) = delete;
    SpscEventQueue& operator=(const SpscEventQueue&) = delete;
    SpscEventQueue(SpscEventQueue&&) = delete;
    SpscEventQueue& operator=(SpscEventQueue&&) = delete;
    ~SpscEventQueue() = default;

    [[nodiscard]] static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    // Only the producer may call this. Returns false instead of overwriting.
    [[nodiscard]] bool try_push(const T& message) noexcept {
        const auto write = write_index_.load(std::memory_order_relaxed);
        const auto next = successor(write);

        if (next == read_index_.load(std::memory_order_acquire)) {
            return false;
        }

        slots_[write] = message;  // Copy complete before publishing.
        write_index_.store(next, std::memory_order_release);
        return true;
    }

    // Only the consumer may call this. A failed pop leaves out unchanged.
    [[nodiscard]] bool try_pop(T& out) noexcept {
        const auto read = read_index_.load(std::memory_order_relaxed);

        if (read == write_index_.load(std::memory_order_acquire)) {
            return false;
        }

        out = slots_[read];  // Read complete before releasing the slot.
        read_index_.store(successor(read), std::memory_order_release);
        return true;
    }

private:
    [[nodiscard]] static constexpr std::size_t successor(
        std::size_t index) noexcept {
        return index + 1 == storage_capacity ? 0 : index + 1;
    }

    // Separate producer and consumer index writes on typical 64-byte lines.
    // Alignment is a performance hint, not a universal cache-line guarantee.
    std::array<T, storage_capacity> slots_{};
    alignas(64) std::atomic<std::size_t> read_index_{0};
    alignas(64) std::atomic<std::size_t> write_index_{0};
};

}  // namespace market_data

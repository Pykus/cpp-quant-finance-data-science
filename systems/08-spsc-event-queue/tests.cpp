#include "spsc_queue.hpp"

#include <atomic>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <thread>

struct Event {
    std::uint64_t sequence{};
    std::int64_t price{};
    std::uint32_t quantity{};
};

static_assert(std::is_trivially_copyable_v<Event>);
static_assert(systems::SpscQueue<Event, 2>::capacity() == 2);
static_assert(systems::SpscQueue<Event, 1024>::capacity() == 1024);

class Checks {
public:
    void verify(bool condition, const char* message) {
        ++count_;
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
    [[nodiscard]] std::size_t count() const noexcept { return count_; }
private:
    std::size_t count_ = 0;
};

void test_empty_and_full(Checks& c) {
    systems::SpscQueue<Event, 2> q;
    Event untouched{99, 77, 55};
    c.verify(!q.try_pop(untouched), "empty pop must fail");
    c.verify(untouched.sequence == 99 && untouched.price == 77 &&
             untouched.quantity == 55, "empty pop must preserve output");
    c.verify(q.try_push(Event{1, 101, 5}), "first enqueue");
    c.verify(q.try_push(Event{2, 102, 6}), "second enqueue");
    c.verify(!q.try_push(Event{3, 103, 7}), "full enqueue must fail");
    Event event{};
    c.verify(q.try_pop(event) && event.sequence == 1 && event.price == 101,
             "full enqueue must not overwrite oldest event");
    c.verify(q.try_pop(event) && event.sequence == 2 && event.quantity == 6,
             "second enqueue intact");
    c.verify(!q.try_pop(event), "drained queue must be empty");
}

void test_wraparound(Checks& c) {
    systems::SpscQueue<Event, 8> q;
    for (std::uint64_t n = 1; n <= 50'000; ++n) {
        c.verify(q.try_push(Event{n, static_cast<std::int64_t>(n * 3), 17}),
                 "enqueue during repeated wraparound");
        Event result{};
        c.verify(q.try_pop(result), "dequeue during repeated wraparound");
        c.verify(result.sequence == n && result.price == static_cast<std::int64_t>(n * 3)
                 && result.quantity == 17, "wraparound must preserve payload");
    }
}

template <std::size_t Capacity>
void test_concurrent(Checks& c) {
    constexpr std::uint64_t total = 100'000;
    systems::SpscQueue<Event, Capacity> q;
    std::atomic<std::uint64_t> errors{0};
    std::atomic<std::uint64_t> consumed{0};

    std::thread consumer([&] {
        Event output{};
        for (std::uint64_t expected = 1; expected <= total;) {
            if (!q.try_pop(output)) {
                std::this_thread::yield();
                continue;
            }
            if (output.sequence != expected ||
                output.price != static_cast<std::int64_t>(expected * 2) ||
                output.quantity != 42) {
                errors.fetch_add(1, std::memory_order_relaxed);
            }
            consumed.store(expected, std::memory_order_relaxed);
            ++expected;
        }
    });

    for (std::uint64_t n = 1; n <= total; ++n) {
        const Event event{n, static_cast<std::int64_t>(n * 2), 42};
        while (!q.try_push(event)) {
            std::this_thread::yield();
        }
    }
    consumer.join();
    c.verify(consumed.load() == total, "consumer count");
    c.verify(errors.load() == 0, "FIFO order and payload under contention");
    Event untouched{123, 456, 789};
    c.verify(!q.try_pop(untouched), "queue empty after concurrent drain");
    c.verify(untouched.sequence == 123 && untouched.price == 456,
             "failed concurrent-drain pop preserves output");
}

int main() {
    try {
        Checks c;
        test_empty_and_full(c);
        test_wraparound(c);
        test_concurrent<2>(c);
        test_concurrent<1024>(c);
        std::cout << "PASS: " << c.count() << "/" << c.count()
                  << " checks; 200000 cross-thread events verified\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}

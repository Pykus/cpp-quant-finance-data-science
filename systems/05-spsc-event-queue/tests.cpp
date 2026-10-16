#include "spsc_event_queue.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>

namespace {

struct MarketEvent {
    std::uint64_t sequence{};
    std::uint64_t price_ticks{};
    std::uint32_t quantity{};
    std::uint8_t side{};
};

static_assert(std::is_trivially_copyable_v<MarketEvent>);
static_assert(!std::is_copy_constructible_v<
    market_data::SpscEventQueue<MarketEvent, 8>>);
static_assert(!std::is_move_constructible_v<
    market_data::SpscEventQueue<MarketEvent, 8>>);

[[nodiscard]] MarketEvent make_event(std::uint64_t sequence) {
    return {sequence, sequence * 7 + 10000,
            static_cast<std::uint32_t>(sequence % 100 + 1),
            static_cast<std::uint8_t>(sequence % 2)};
}

[[nodiscard]] bool same(const MarketEvent& left, const MarketEvent& right) {
    return left.sequence == right.sequence
        && left.price_ticks == right.price_ticks
        && left.quantity == right.quantity
        && left.side == right.side;
}

struct TestSuite {
    std::size_t checks = 0;

    void check(bool condition, const std::string& description) {
        ++checks;
        if (!condition) {
            throw std::runtime_error("Check failed: " + description);
        }
    }
};

void test_single_slot(TestSuite& test) {
    using Queue = market_data::SpscEventQueue<MarketEvent, 1>;
    Queue queue;
    test.check(Queue::capacity() == 1, "capacity one");
    MarketEvent output = make_event(500);
    test.check(!queue.try_pop(output), "initially empty");
    test.check(same(output, make_event(500)),
               "empty pop leaves output untouched");

    test.check(queue.try_push(make_event(1)), "first push succeeds");
    test.check(!queue.try_push(make_event(2)), "full push fails");
    test.check(queue.try_pop(output), "pop from full queue");
    test.check(same(output, make_event(1)), "failed push never overwrites");
    test.check(!queue.try_pop(output), "queue empty after pop");
    test.check(same(output, make_event(1)), "second empty pop keeps output");
    test.check(queue.try_push(make_event(3)), "slot reused");
    test.check(queue.try_pop(output), "pop after slot reuse");
    test.check(same(output, make_event(3)), "reused slot has new message");
}

void test_fifo_and_wraparound(TestSuite& test) {
    using Queue = market_data::SpscEventQueue<MarketEvent, 3>;
    Queue queue;
    test.check(Queue::capacity() == 3, "non-power-of-two capacity");
    MarketEvent output{};
    for (std::uint64_t round = 0; round < 40; ++round) {
        for (std::uint64_t slot = 1; slot <= 3; ++slot) {
            test.check(queue.try_push(make_event(3 * round + slot)),
                       "push before wraparound");
        }
        test.check(!queue.try_push(make_event(999)),
                   "capacity limit after wraparound");
        for (std::uint64_t slot = 1; slot <= 3; ++slot) {
            test.check(queue.try_pop(output), "pop before wraparound");
            test.check(same(output, make_event(3 * round + slot)),
                       "FIFO preserves entire message");
        }
        test.check(!queue.try_pop(output), "empty after each round");
    }

    test.check(queue.try_push(make_event(400)), "partial fill 1");
    test.check(queue.try_push(make_event(401)), "partial fill 2");
    test.check(queue.try_pop(output), "partial pop");
    test.check(same(output, make_event(400)), "partial pop value");
    test.check(queue.try_push(make_event(402)), "interleaved push");
    test.check(queue.try_push(make_event(403)), "interleaved fill");
    test.check(!queue.try_push(make_event(404)), "interleaved full");
    for (std::uint64_t id = 401; id <= 403; ++id) {
        test.check(queue.try_pop(output), "interleaved pop");
        test.check(same(output, make_event(id)), "interleaved FIFO");
    }
}

void test_separate_queues(TestSuite& test) {
    market_data::SpscEventQueue<MarketEvent, 2> left;
    market_data::SpscEventQueue<MarketEvent, 2> right;
    MarketEvent out{};
    test.check(left.try_push(make_event(100)), "left push");
    test.check(!right.try_pop(out), "right independent empty");
    test.check(right.try_push(make_event(200)), "right push");
    test.check(left.try_pop(out), "left pop");
    test.check(same(out, make_event(100)), "left independent contents");
    test.check(right.try_pop(out), "right pop");
    test.check(same(out, make_event(200)), "right independent contents");
}

void test_against_reference_deque(TestSuite& test) {
    market_data::SpscEventQueue<MarketEvent, 7> queue;
    std::deque<MarketEvent> reference;
    std::minstd_rand random(1234567);
    std::uint64_t next_sequence = 1;
    MarketEvent output{};

    for (std::size_t operation = 0; operation < 6000; ++operation) {
        const bool push = (random() % 3) != 0;
        if (push) {
            const bool expected = reference.size() < 7;
            const auto message = make_event(next_sequence++);
            const bool observed = queue.try_push(message);
            test.check(observed == expected, "reference capacity comparison");
            if (expected) {
                reference.push_back(message);
            }
        } else {
            const bool expected = !reference.empty();
            const bool observed = queue.try_pop(output);
            test.check(observed == expected, "reference empty comparison");
            if (expected) {
                test.check(same(output, reference.front()),
                           "reference FIFO comparison");
                reference.pop_front();
            }
        }
    }
    while (!reference.empty()) {
        test.check(queue.try_pop(output), "final drain");
        test.check(same(output, reference.front()), "final drain order");
        reference.pop_front();
    }
    test.check(!queue.try_pop(output), "reference drained completely");
}

void test_concurrent_producer_consumer(TestSuite& test) {
    market_data::SpscEventQueue<MarketEvent, 64> queue;
    constexpr std::uint64_t total = 100000;
    const auto deadline = std::chrono::steady_clock::now()
                        + std::chrono::seconds(20);
    std::atomic<bool> valid{true};
    std::atomic<bool> timeout{false};
    std::uint64_t produced = 0;
    std::uint64_t consumed = 0;

    std::thread producer([&] {
        for (std::uint64_t sequence = 1; sequence <= total; ++sequence) {
            const auto event = make_event(sequence);
            while (!queue.try_push(event)) {
                if (std::chrono::steady_clock::now() > deadline) {
                    timeout.store(true, std::memory_order_relaxed);
                    return;
                }
                std::this_thread::yield();
            }
            ++produced;
        }
    });

    std::thread consumer([&] {
        for (std::uint64_t sequence = 1; sequence <= total; ++sequence) {
            MarketEvent event{};
            while (!queue.try_pop(event)) {
                if (std::chrono::steady_clock::now() > deadline) {
                    timeout.store(true, std::memory_order_relaxed);
                    return;
                }
                std::this_thread::yield();
            }
            if (!same(event, make_event(sequence))) {
                valid.store(false, std::memory_order_relaxed);
            }
            ++consumed;
        }
    });

    producer.join();
    consumer.join();
    test.check(!timeout.load(), "cross-thread execution before timeout");
    test.check(valid.load(), "all fields visible in proper sequence");
    test.check(produced == total, "producer processed every event");
    test.check(consumed == total, "consumer processed every event");
    MarketEvent out{};
    test.check(!queue.try_pop(out), "queue empty after cross-thread drain");
}

}  // namespace

int main() {
    try {
        TestSuite tests;
        test_single_slot(tests);
        test_fifo_and_wraparound(tests);
        test_separate_queues(tests);
        test_against_reference_deque(tests);
        test_concurrent_producer_consumer(tests);
        std::cout << "Passed " << tests.checks
                  << " checks across five test scenarios.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

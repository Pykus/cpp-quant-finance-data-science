#include "bounded_queue.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {
int checks = 0;
int failures = 0;

void check(bool ok, const char* label) {
    ++checks;
    if (!ok) {
        ++failures;
        std::cerr << "FAILED: " << label << "\n";
    }
}
}  // namespace

int main() {
    using systems::BoundedQueue;
    using namespace std::chrono_literals;

    bool invalid_capacity = false;
    try {
        BoundedQueue<int> invalid(0);
    } catch (const std::invalid_argument&) {
        invalid_capacity = true;
    }
    check(invalid_capacity, "capacity zero rejected");

    BoundedQueue<int> queue(2);
    check(queue.capacity() == 2, "capacity accessor");
    check(queue.size() == 0, "initially empty");
    check(!queue.try_pop().has_value(), "no element to poll");
    check(queue.try_push(11), "first enqueue");
    check(queue.try_push(22), "second enqueue");
    check(queue.size() == 2, "queue full");
    check(!queue.try_push(33), "full queue declines immediate push");
    check(queue.try_pop().value() == 11, "FIFO first");
    check(queue.pop().value() == 22, "FIFO second");
    check(queue.size() == 0, "drained");

    BoundedQueue<std::unique_ptr<int>> move_only(1);
    check(move_only.push(std::make_unique<int>(42)), "move-only insertion");
    const auto moved = move_only.pop();
    check(moved.has_value() && *moved.value() == 42, "move-only retrieval");
    move_only.close();
    check(!move_only.pop().has_value(), "closed empty returns nullopt");

    BoundedQueue<int> shutdown(1);
    check(shutdown.push(7), "pre-close insertion");
    shutdown.close();
    shutdown.close();
    check(shutdown.is_closed(), "close idempotent");
    check(!shutdown.push(8), "blocking push after close returns false");
    check(!shutdown.try_push(9), "immediate push after close returns false");
    check(shutdown.pop().value() == 7, "close permits draining");
    check(!shutdown.pop().has_value(), "closed and drained");

    // A consumer waiting for work must wake when the queue is closed.
    BoundedQueue<int> waiting_consumer(1);
    auto waiting = std::async(std::launch::async,
                              [&waiting_consumer] { return waiting_consumer.pop(); });
    waiting_consumer.close();
    check(waiting.wait_for(5s) == std::future_status::ready, "consumer awakened");
    check(!waiting.get().has_value(), "consumer received close signal");

    // A producer blocked by backpressure must also wake on close.
    BoundedQueue<int> waiting_producer(1);
    check(waiting_producer.push(1), "fill queue for blocked producer");
    auto blocked = std::async(std::launch::async,
                               [&waiting_producer] { return waiting_producer.push(2); });
    waiting_producer.close();
    check(blocked.wait_for(5s) == std::future_status::ready, "producer awakened");
    check(!blocked.get(), "blocked producer refused after close");
    check(waiting_producer.pop().value() == 1, "stored element still present");

    // Verify that multiple producers and consumers lose or duplicate no IDs.
    constexpr int producer_count = 4;
    constexpr int consumer_count = 3;
    constexpr int per_producer = 2000;
    constexpr int total = producer_count * per_producer;
    BoundedQueue<int> concurrent(17);
    std::vector<std::atomic<int>> seen(total);
    for (auto& counter : seen) {
        counter.store(0);
    }
    std::atomic<int> failed_pushes{0};
    std::vector<std::thread> consumers;
    for (int worker = 0; worker < consumer_count; ++worker) {
        consumers.emplace_back([&] {
            while (auto value = concurrent.pop()) {
                if (*value >= 0 && *value < total) {
                    seen[static_cast<std::size_t>(*value)].fetch_add(1);
                }
            }
        });
    }

    std::vector<std::thread> producers;
    for (int worker = 0; worker < producer_count; ++worker) {
        producers.emplace_back([&, worker] {
            for (int i = 0; i < per_producer; ++i) {
                if (!concurrent.push(worker * per_producer + i)) {
                    failed_pushes.fetch_add(1);
                }
            }
        });
    }
    for (auto& producer : producers) {
        producer.join();
    }
    concurrent.close();
    for (auto& consumer : consumers) {
        consumer.join();
    }
    check(failed_pushes.load() == 0, "no rejected producer work");
    check(concurrent.is_closed(), "concurrent shutdown");
    check(concurrent.size() == 0, "concurrent drain");
    for (const auto& count : seen) {
        check(count.load() == 1, "exactly one consumption per ID");
    }

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}

#include "spsc_queue.hpp"

#include <cstdint>
#include <iostream>
#include <thread>

struct MarketEvent {
    std::uint64_t sequence;
    std::int64_t price_ticks;
    std::uint32_t quantity;
};

int main() {
    constexpr std::uint64_t event_count = 100'000;
    systems::SpscQueue<MarketEvent, 1024> queue;
    std::uint64_t received = 0;
    bool in_order = true;

    std::thread consumer([&] {
        MarketEvent event{};
        for (std::uint64_t expected = 1; expected <= event_count;) {
            if (!queue.try_pop(event)) {
                std::this_thread::yield();
                continue;
            }
            if (event.sequence != expected ||
                event.price_ticks != 10'000 + static_cast<std::int64_t>(expected) ||
                event.quantity != 100) {
                in_order = false;
            }
            received = expected;
            ++expected;
        }
    });

    for (std::uint64_t sequence = 1; sequence <= event_count; ++sequence) {
        const MarketEvent event{sequence, 10'000 + static_cast<std::int64_t>(sequence), 100};
        while (!queue.try_push(event)) {
            std::this_thread::yield();
        }
    }

    consumer.join();
    std::cout << "Market events received: " << received << '\n';
    std::cout << "Sequence and payload checks: " << (in_order ? "PASS" : "FAIL") << '\n';
    return in_order && received == event_count ? 0 : 1;
}

#include "spsc_event_queue.hpp"

#include <cstdint>
#include <iostream>

namespace {

struct MarketEvent {
    std::uint64_t sequence{};
    std::uint64_t price_ticks{};
    std::uint32_t quantity{};
    std::uint8_t side{};  // 0: buy, 1: sell
};

}  // namespace

int main() {
    // Imagine a network thread producing feed updates while one matching or
    // analytics thread consumes them. Here we simulate both roles in one
    // thread to make queue behavior easy to see.
    market_data::SpscEventQueue<MarketEvent, 3> events;

    for (std::uint64_t sequence = 1; sequence <= 3; ++sequence) {
        const MarketEvent message{
            sequence, 10000 + 25 * sequence,
            static_cast<std::uint32_t>(sequence * 2), 0};
        if (!events.try_push(message)) {
            std::cerr << "Unexpected capacity error\n";
            return 1;
        }
    }

    const MarketEvent fourth{4, 10100, 8, 1};
    if (!events.try_push(fourth)) {
        std::cout << "Queue full: event 4 must wait (backpressure).\n";
    }

    MarketEvent received{};
    while (events.try_pop(received)) {
        std::cout << "Received #" << received.sequence
                  << " price_ticks=" << received.price_ticks
                  << " quantity=" << received.quantity << '\n';
    }
    std::cout << "All three queued events processed in FIFO order.\n";
}

#include "spsc_queue.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

struct Event {
    std::uint64_t id;
    std::uint64_t price;
};

int main() {
    constexpr std::size_t iterations = 100'000;
    systems::SpscQueue<Event, 1024> queue;
    std::vector<std::int64_t> durations;
    durations.reserve(iterations);
    std::uint64_t checksum = 0;

    for (std::size_t i = 0; i < iterations; ++i) {
        Event output{};
        const auto begin = std::chrono::steady_clock::now();
        const bool pushed = queue.try_push(Event{i + 1, i + 100});
        const bool popped = queue.try_pop(output);
        const auto end = std::chrono::steady_clock::now();
        if (!pushed || !popped || output.id != i + 1) {
            std::cerr << "Queue invariant failed\n";
            return 1;
        }
        checksum += output.price;
        durations.push_back(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count());
    }

    std::sort(durations.begin(), durations.end());
    const auto at_percentile = [&](std::size_t numerator) {
        const std::size_t rank = (iterations * numerator + 99) / 100;
        return durations[rank - 1];
    };

    std::cout << "Uncontended enqueue+dequeue pair (nanoseconds)\n"
              << "p50=" << at_percentile(50) << " p99=" << at_percentile(99)
              << " samples=" << iterations << " checksum=" << checksum << '\n';
}

#include "bounded_queue.hpp"

#include <iostream>
#include <thread>
#include <vector>

int main() {
    systems::BoundedQueue<int> events(3);

    // One producer models an incoming event feed.
    std::thread producer([&events] {
        for (int event = 1; event <= 5; ++event) {
            if (!events.push(event)) {
                return;
            }
        }
        events.close();
    });

    // Two workers process events until the queue is closed and drained.
    std::vector<std::thread> workers;
    for (int worker = 0; worker < 2; ++worker) {
        workers.emplace_back([&events] {
            while (auto item = events.pop()) {
                // In a real program: parse or persist the event.
                (void)*item;
            }
        });
    }

    producer.join();
    for (auto& worker : workers) {
        worker.join();
    }
    std::cout << "All 5 synthetic events dispatched and workers stopped.\n";
}

#include "token_bucket.hpp"

#include <chrono>
#include <iostream>
#include <string_view>

int main() {
    using namespace std::chrono_literals;
    using systems::TokenBucket;

    const auto start = TokenBucket::TimePoint{};
    TokenBucket limiter{3, 2, start};  // Burst of 3, then 2 events/second.

    const auto attempt = [&](std::string_view name,
                             std::chrono::milliseconds elapsed) {
        const bool accepted = limiter.try_acquire(1, start + elapsed);
        std::cout << name << " at " << elapsed.count() << " ms: "
                  << (accepted ? "admitted" : "rejected") << '\n';
    };

    attempt("Event A", 0ms);
    attempt("Event B", 0ms);
    attempt("Event C", 0ms);
    attempt("Event D", 0ms);
    attempt("Event E", 500ms);
    attempt("Event F", 750ms);
    attempt("Event G", 1000ms);
}

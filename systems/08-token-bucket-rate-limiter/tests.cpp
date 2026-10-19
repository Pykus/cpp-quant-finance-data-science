#include "token_bucket.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {
std::uint64_t checks = 0;

void check(bool condition, const char* description) {
    ++checks;
    if (!condition) {
        throw std::runtime_error(std::string{"FAILED: "} + description);
    }
}
}  // namespace

int main() {
    using namespace std::chrono_literals;
    using systems::TokenBucket;
    const TokenBucket::TimePoint epoch{};

    try {
        {
            TokenBucket bucket{3, 2, epoch};
            check(bucket.capacity() == 3, "capacity accessor");
            check(bucket.refill_rate() == 2, "rate accessor");
            check(bucket.available(epoch) == 3, "starts full");
            check(bucket.try_acquire(1, epoch), "first token");
            check(bucket.try_acquire(2, epoch), "remaining tokens");
            check(!bucket.try_acquire(1, epoch), "empty rejects");
            check(bucket.available(epoch + 499ms) == 0, "no early refill");
            check(bucket.available(epoch + 500ms) == 1, "exact half-second");
            check(bucket.try_acquire(1, epoch + 500ms), "use refilled token");
            check(bucket.available(epoch + 999ms) == 0, "fraction carried");
            check(bucket.available(epoch + 1000ms) == 1, "one second total");
            check(!bucket.try_acquire(4, epoch + 1000ms),
                  "cannot exceed burst capacity");
            check(bucket.try_acquire(0, epoch + 1000ms),
                  "zero request is a no-op");
            check(bucket.available(epoch + 1000ms) == 1,
                  "zero request preserved token");
        }
        {
            TokenBucket bucket{2, 1, epoch};
            check(bucket.try_acquire(2, epoch), "empty at initial instant");
            check(bucket.available(epoch + 999ms) == 0, "999ms has no token");
            check(bucket.available(epoch + 1000ms) == 1, "1000ms has one");
            check(bucket.available(epoch + 2500ms) == 2, "saturate capacity");
            check(bucket.available(epoch + 10000000s) == 2,
                  "long interval must safely saturate");
        }
        {
            TokenBucket bucket{2, 2, epoch};
            check(bucket.try_acquire(1, epoch), "consume one");
            check(bucket.available(epoch + 500ms) == 2, "refill to full");
            check(bucket.available(epoch + 750ms) == 2, "full bucket remains full");
            check(bucket.try_acquire(2, epoch + 750ms), "drain after full");
            check(bucket.available(epoch + 1000ms) == 0,
                  "cannot bank fractional credit while full");
            check(bucket.available(epoch + 1250ms) == 1,
                  "refill resumes after becoming non-full");
        }
        {
            TokenBucket bucket{3, 3, epoch};
            check(bucket.try_acquire(3, epoch), "drain fractional test");
            check(bucket.available(epoch + 333ms) == 0, "333ms not enough");
            check(bucket.available(epoch + 334ms) == 1, "fraction crosses one");
            check(bucket.try_acquire(1, epoch + 334ms), "spend first accrued");
            check(bucket.available(epoch + 667ms) == 1, "fraction remainder kept");
            check(bucket.try_acquire(1, epoch + 667ms), "spend second accrued");
            check(bucket.available(epoch + 999ms) == 0, "fraction before third");
            check(bucket.available(epoch + 1000ms) == 1, "third accrued");
        }
        {
            TokenBucket bucket{1, 1000, epoch};
            check(bucket.try_acquire(1, epoch), "drain precise bucket");
            check(bucket.available(epoch + 999us) == 0,
                  "less than a millisecond never refills");
            check(bucket.available(epoch + 1000us) == 1,
                  "millisecond boundary refills");
            check(bucket.try_acquire(1, epoch + 1000us),
                  "spend at millisecond boundary");
            check(bucket.available(epoch + 1999us) == 0,
                  "sub-ms remainder is preserved");
            check(bucket.available(epoch + 2000us) == 1,
                  "second millisecond boundary");
        }
        {
            TokenBucket bucket{3, 2, epoch};
            check(bucket.try_acquire(3, epoch), "consume before backwards timestamp");
            check(bucket.available(epoch - 100ms) == 0,
                  "backward timestamp has no credit");
            check(bucket.available(epoch + 500ms) == 1,
                  "backward time did not reset reference timestamp");
        }
        {
            TokenBucket bucket{1'000'000, 1'000'000, epoch};
            check(bucket.try_acquire(1'000'000, epoch), "drain upper bound");
            check(bucket.available(epoch + 10ms) == 10'000,
                  "upper-rate millisecond arithmetic");
            check(bucket.available(epoch + 1000ms) == 1'000'000,
                  "upper bound refill saturates");
        }
        {
            for (const auto [cap, rate] : {
                     std::pair{0u, 1u},
                     std::pair{1u, 0u},
                     std::pair{1'000'001u, 1u},
                     std::pair{1u, 1'000'001u}}) {
                bool threw = false;
                try {
                    TokenBucket bucket{cap, rate, epoch};
                    (void)bucket;
                } catch (const std::invalid_argument&) {
                    threw = true;
                }
                check(threw, "invalid configuration rejected");
            }
        }
        {
            // 100 seconds of one event request per ms, 100-token/sec rate.
            // The loop checks that emitted traffic stays within the budget.
            TokenBucket bucket{10, 100, epoch};
            std::uint64_t accepted = 0;
            for (std::int64_t ms = 0; ms < 100'000; ++ms) {
                if (bucket.try_acquire(1, epoch + std::chrono::milliseconds{ms})) {
                    ++accepted;
                }
                check(bucket.available(
                    epoch + std::chrono::milliseconds{ms}) <= 10,
                    "always bounded even under sustained traffic");
            }
            check(accepted >= 10'000 && accepted <= 10'010,
                  "rate is enforced over 100 seconds");
            std::cout << "Accepted requests during 100-second stress run: "
                      << accepted << '\n';
        }
        std::cout << checks << " checks passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

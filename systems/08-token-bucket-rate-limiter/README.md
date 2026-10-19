# C++20 Systems: Bounded Token Bucket Rate Limiter

## A real systems problem

An application receives a burst of events from a network connection: telemetry packets, log messages, market-data subscriptions or API requests. The receiver can handle **2 requests per second**, but it should tolerate a small burst rather than reject everything immediately.

A naive rule such as "sleep 500 ms after every event" adds unnecessary delay and blocks the processing thread. A **token bucket** is a non-blocking way to decide whether to admit or reject each request.

This project implements a small, predictable, integer-based token bucket in modern C++20. It contains no sockets, allocations in the hot path, background threads or hidden clock reads. The caller supplies the current monotonic time.

## How the example works

Configuration: burst capacity **3 tokens**; refill rate **2 tokens per second**; each accepted event costs **1 token**.

| Time | Attempt | Decision | Explanation |
| --- | --- | --- | --- |
| 0 ms | A | admit | Initially 3 tokens |
| 0 ms | B | admit | 2 tokens remain before request |
| 0 ms | C | admit | Last initial token |
| 0 ms | D | reject | Bucket is empty |
| 500 ms | E | admit | Exactly one token refilled |
| 750 ms | F | reject | Only half a token accrued |
| 1000 ms | G | admit | Fractional credit reaches a full token |

Try it:

~~~bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o limiter_demo
./limiter_demo
~~~

Expected console output:

~~~text
Event A at 0 ms: admitted
Event B at 0 ms: admitted
Event C at 0 ms: admitted
Event D at 0 ms: rejected
Event E at 500 ms: admitted
Event F at 750 ms: rejected
Event G at 1000 ms: admitted
~~~

## Test it

~~~bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o limiter_tests
./limiter_tests
~~~

The tests check exact refill boundaries, fractional token carry, requests over the capacity, saturation after long idle periods, non-monotonic timestamps, sub-millisecond rounding, invalid configuration, and **100,000 consecutive event attempts**.

Optional sanitizer check:

~~~bash
g++ -std=c++20 -g -O1 -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o limiter_tests_san
./limiter_tests_san
~~~

No third-party dependencies. The algorithm is written in a header for easy reuse.

## Use it in a service

~~~cpp
#include "token_bucket.hpp"

// Create this once when your service starts:
systems::TokenBucket limiter{200, 100, systems::TokenBucket::Clock::now()};

// At each event, on the same thread:
if (limiter.try_acquire(1, systems::TokenBucket::Clock::now())) {
    // Process the event.
} else {
    // Drop, defer or record a rate-limit event.
}
~~~

The constructor defines **200 tokens max** and **100 new tokens per second**. Asking for N tokens spends N from the same budget. Invalid capacity or rate is rejected at construction (allowed range 1–1,000,000).

## Engineering choices and limitations

- **Integer arithmetic:** Uses whole milliseconds and fractional milli-tokens, not floating-point. Unused sub-millisecond time is carried forward.
- **Bounded burst:** When full, the bucket discards excess refill credit instead of banking it indefinitely.
- **Explicit monotonic clock:** Tests can simulate time without sleeping. Backwards timestamps never create credits.
- **Fast path:** No allocation, logging, exceptions or blocking operations in try_acquire().
- **Single threaded:** A TokenBucket instance is **not thread-safe**. Use one instance per thread or guard shared access with synchronization.
- **Not a general API gateway:** No network integration, persistent state, distributed quotas, fairness across clients, or retry queue. The accepted count may vary slightly at millisecond boundaries; the program deliberately trades sub-millisecond precision for simple bounded arithmetic.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for a worked arithmetic explanation, and compare this with [TCP stream framing](../07-tcp-stream-framer/) for another independent networking building block.

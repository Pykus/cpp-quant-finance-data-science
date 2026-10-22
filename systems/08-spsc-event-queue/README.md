# C++20 Systems — Bounded SPSC Market Event Queue

## Problem

A market-data thread decodes synthetic exchange events while a second thread processes them. Passing every event through a mutex-protected, dynamically allocated container adds synchronization and allocation work. A **single-producer/single-consumer (SPSC)** ring is a small, bounded alternative when exactly one thread produces and exactly one other thread consumes.

This lesson implements an owning, fixed-capacity queue of trivially copyable events. It is built for learning the C++20 memory model and reproducible testing, not as a drop-in production exchange component.

## API and invariants

```cpp
#include "spsc_queue.hpp"

struct OrderEvent {
    std::uint64_t sequence;
    std::int64_t price_ticks;
    std::uint32_t quantity;
};

systems::SpscQueue<OrderEvent, 1024> events;
const bool accepted = events.try_push(OrderEvent{1, 12345, 100});
OrderEvent next{};
const bool received = events.try_pop(next);
```

- `Capacity` is a compile-time power of two, at least 2; **all Capacity slots are usable**.
- `try_push()` returns `false` if full. No event is dropped silently or overwritten.
- `try_pop()` returns `false` if empty and leaves its output unchanged.
- Successful operations preserve FIFO order and the entire event payload.
- **Exactly one producer thread** may call `try_push` and **exactly one consumer thread** may call `try_pop`. Multiple producers or multiple consumers violate the contract.
- Neither hot-path operation dynamically allocates, waits on a mutex, or throws; preallocated storage exists for the queue lifetime.
- Event types must be trivially copyable, default-constructible and nothrow copy-assignable. Use scaled integer price ticks rather than floating-point currency.
- Do not destroy the queue while either worker is using it; join both workers first.

`head` is written by the producer and `tail` by the consumer. Each lives in a separate 64-byte-aligned object to *reduce* false sharing on common hardware; this is not a guarantee of a particular machine's cache line size. A release-store publishes the written payload; an acquire-load lets the other thread see it. This ordering also prevents the producer from overwriting a slot before the consumer finishes reading it.

**Why power-of-two capacity?** `index & (Capacity - 1)` maps monotonically increasing unsigned counters onto the ring, including after counter wrap. Distances use unsigned subtraction and are valid under the bounded SPSC invariant. Non-power-of-two capacities are intentionally rejected at compile time.

## Build and run

Requires a C++20 compiler with `<atomic>` and a standard threads implementation.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/queue_demo
./build/queue_benchmark
```

Standalone Linux commands:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread tests.cpp -o queue_tests
./queue_tests
```

The demo passes **100,000 ordered synthetic market events** between two threads. Tests cover empty/full behavior, output preservation, repeated wraparound, and **200,000 threaded event transfers** with capacities 2 and 1024. Checks are counted in the test executable's output.

## Benchmark: what it does and does not tell you

`benchmark.cpp` samples **100,000** enqueue+dequeue pairs on a **single thread** and prints empirical p50 and p99 sample durations in nanoseconds. It uses `std::chrono::steady_clock` and checks a payload checksum. Measurements include clock overhead and are sensitive to CPU frequency, scheduler interrupts and optimizer decisions. They are **not cross-thread handoff latency**, cannot meaningfully compare machines without controls, and do not establish a particular latency service-level objective.

For defensible low-latency work, additionally measure actual producer-to-consumer timestamps under load, pin cores deliberately, warm up, report CPU/cache/topology, include p99.9, monitor queue occupancy and failed pushes, and run repeated trials. Each consumer must eventually process or deliberately discard events according to a documented backpressure policy.

## Threading and limitations

- Busy-spinning in tests and demo is only for deterministic finite synthetic workloads. A live service needs shutdown, backpressure, observability, and possibly bounded waiting.
- The queue does not know when production is complete. Real callers should use a separate completion protocol after all buffered items have drained.
- This example supports SPSC only; MPSC/MPMC requires a different algorithm.
- It does not guarantee `std::atomic<std::size_t>` is lock-free on every architecture. Verify `is_lock_free()` on the deployment target if that matters.
- ThreadSanitizer is a useful *additional* race-detection step where supported; passing ASan/UBSan alone does not prove the absence of data races.
- No persistence, network transport, retransmission, or multi-process shared memory is provided.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for an intuitive walk through data ownership, the ring indices and acquire/release synchronization.

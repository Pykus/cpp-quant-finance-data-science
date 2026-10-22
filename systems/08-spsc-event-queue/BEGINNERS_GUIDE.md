# Beginner's Guide: A Two-Thread Event Queue in C++20

## 1. What are we building?

Imagine an application receiving market prices. A network thread parses a message into an event, while a processing thread updates a local order book. Both have different jobs. We need to hand over each event exactly once and in the same order.

Think of a queue at a supermarket: the first person to arrive is the first served. In computer science this is **FIFO**, first in, first out.

Our event is small:

```cpp
struct MarketEvent {
    std::uint64_t sequence;
    std::int64_t price_ticks;
    std::uint32_t quantity;
};
```

`sequence` allows us to detect missing or reordered events. `price_ticks` is an integer (for example 12345 ticks). Integer ticks avoid ambiguities that floating-point amounts could introduce. `quantity` counts a fictional traded amount.

## 2. Why not `std::queue`?

`std::queue` provides FIFO operations but not automatic inter-thread synchronization. Calling `push` and `pop` concurrently without protection is a data race and therefore undefined behavior. You can protect it with a `std::mutex`, which is often the simplest correct solution.

For high-frequency events, we can investigate a deliberately narrower case: **one producer and one consumer only**, sharing a finite array. That makes it possible to coordinate using two counters and atomics.

Important: this lesson does not claim mutexes are universally slow. In a real application you should benchmark a correct mutex baseline too.

## 3. Why a ring?

Picture a row of four slots:

```text
slot index      0      1      2      3
contents       A      B      C      D
```

The producer fills slots in order. After the fourth slot it starts again at slot zero, *but only when the consumer has already released that slot*. This is called a circular buffer or ring.

Two ever-increasing conceptual counters track progress:

- `head`: how many events the producer has published.
- `tail`: how many events the consumer has removed.

The number of buffered events is `head - tail`. We use **unsigned** counters, so subtraction works even if machine-size integers eventually wrap, provided the ring occupancy invariant remains true.

For capacity 4:

```text
head  tail  buffered  interpretation
0     0       0       empty
1     0       1       one published event
4     0       4       full
4     1       3       one slot freed
5     1       4       full again
```

Because our capacity is a power of two, we compute the actual array index with `head & (Capacity - 1)`, which is equivalent to `head % Capacity` for the supported capacities.

## 4. Full and empty without wasting a slot

The queue is empty if `tail == head`. It is full if `head - tail == Capacity`.

Some ring designs store only indices modulo capacity. Such designs cannot tell full from empty without an extra flag, so they sometimes leave one slot unused. **Our counters keep the progress count**, meaning a queue with capacity 1024 can really store 1024 events.

If `try_push` sees full, it returns false. It does not overwrite an older event. If `try_pop` sees empty, it returns false and leaves your output variable intact.

Decide at the caller whether to wait, count dropped events, report backpressure, or shut down. Never silently discard financial events if your protocol requires complete sequence tracking.

## 5. Why ordinary integers are insufficient

The producer and consumer run at different times on different CPU cores. Modern CPUs and compilers may reorder memory operations when no synchronization forbids it.

Without synchronization, one thread could observe an updated `head` counter but not the data intended for that slot. Accessing a shared non-atomic counter concurrently is also a data race in C++.

Use `std::atomic<std::size_t>` for the counters. Atomics are about both safe simultaneous access and the *ordering* of memory effects.

## 6. Four memory-order operations

Producer logic, simplified:

```cpp
const auto head = head_.load(std::memory_order_relaxed);
const auto tail = tail_.load(std::memory_order_acquire);
if (head - tail == Capacity) return false;
buffer_[head & (Capacity - 1)] = event;
head_.store(head + 1, std::memory_order_release);
```

Consumer logic:

```cpp
const auto tail = tail_.load(std::memory_order_relaxed);
const auto head = head_.load(std::memory_order_acquire);
if (tail == head) return false;
event = buffer_[tail & (Capacity - 1)];
tail_.store(tail + 1, std::memory_order_release);
```

The same thread writes only its own counter. Therefore its own counter's load can be **relaxed**: it needs an atomic value, but not a new synchronizes-with relationship.

After writing the event, the producer performs a **release** store to `head`. When the consumer's **acquire** load sees that publication (or a later one from the same producer), the write to the event is visible to it. This release/acquire pair establishes a *happens-before* relationship.

In reverse, after reading the event the consumer release-stores a new `tail`. The producer acquire-loads `tail` before reusing that slot. This ensures the consumer has finished reading before the producer overwrites it.

**`memory_order_relaxed` is not universally safe just because an object is atomic.** A queue using relaxed operations for publication could break the necessary visibility ordering.

## 7. What is false sharing?

CPUs transfer memory in chunks called **cache lines**, commonly 64 bytes on desktop/server processors. If two cores repeatedly update different variables on the same line, they may still force that line to migrate between cores. That is false sharing.

The producer changes `head`; the consumer changes `tail`. We align them in separate 64-byte-sized/aligned objects to reduce the chance they share a cache line. Actual cache-line sizes vary by architecture. Alignment itself does not make a non-atomic operation thread-safe.

## 8. Walkthrough: push and pop

Start with capacity two:

```text
head=0 tail=0   [] []       empty
push A          [A] []      head=1, tail=0
push B          [A] [B]     head=2, tail=0, full
push C          returns false; A and B remain untouched
pop             returns A;  head=2, tail=1
push C          [C] [B]     head=3, tail=1
pop             returns B;  head=3, tail=2
pop             returns C;  head=3, tail=3, empty
```

The third `push` succeeds *only after* the first `pop` publishes its new `tail`.

## 9. Tests that matter

A correct-looking example is not enough for a concurrent data structure. The included tests exercise:

1. `pop` from empty and unchanged output.
2. `push` until full, failed `push`, and intact old contents.
3. Repeated wraparound with full payload comparisons.
4. A **two-slot ring** transferring 100,000 events between two threads.
5. A **1024-slot ring** transferring another 100,000 events.
6. Both thread runs verify order, price and quantity, then check the drained state.

There is a limitation: passing finite stress tests does not mathematically prove the implementation correct. Read the memory-order argument, review the SPSC contract, and use additional tools such as ThreadSanitizer where available.

Run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## 10. Understanding latency numbers

A *benchmark* makes many measurements to estimate how long operations take. The executable here measures one enqueue and dequeue pair within the same thread and reports:

- `p50`: the median sampled pair duration. Roughly half of samples are at or below it.
- `p99`: the sample value around the 99th percentile. Roughly 99% of samples are at or below it.

A pair may take more time because the operating system interrupted execution, not because the algorithm changed. Reading a clock takes time too, so tiny measurements can be dominated by clock resolution and overhead. The benchmark does **not** measure transfer time between separate cores.

To compare approaches, run both under similar conditions and measure p50, p99 and p99.9 over repeated trials. Record machine characteristics outside the public codebase if they identify private infrastructure.

## 11. What would a real service add?

A production-grade feed handler must define what happens when the queue fills. Busy-waiting indefinitely can burn a CPU core, and dropping exchange events without detection can corrupt downstream state. Strategies include admission control, explicit backpressure, bounded retry, sequence-gap detection and failover.

It also needs a shutdown protocol, telemetry (queue depth, unsuccessful pushes, sequence gaps), overload tests, hardware-specific benchmarking, and thread-affinity decisions. If you need two producer threads, stop here: **this is not an MPSC queue**.

## 12. Exercises

1. Draw a capacity-four queue after 6 pushes and 3 pops, assuming the producer waits on full. Which slots contain the remaining events?
2. Why is `std::atomic` needed even when only one thread writes each counter?
3. What exactly does the producer's `release` store protect?
4. Why does the producer also need an `acquire` load of the consumer's `tail`?
5. Change the demo to capacity two. Does it still produce all 100,000 events in order?
6. Add a counter for failed `try_push` attempts, then explain why that count is workload- and scheduler-dependent.
7. Implement a mutex-and-condition-variable alternative. Which design is easier to shut down reliably?
8. Outline a benchmark for **actual cross-thread handoff latency** without modifying the SPSC ownership rules.

### Suggested answers to conceptual questions

**Question 2:** Reads of a shared non-atomic counter concurrent with another thread's writes would be a C++ data race; the atomics also provide memory ordering.

**Question 3:** It publishes prior writes to an event before a consumer whose acquire load observes the new head may read that event.

**Question 4:** Reclaiming a slot must not race the consumer's previous read. The consumer's release store of `tail` pairs with the producer's acquire load of it.

**Question 5:** Yes, but the producer is likely to retry more often. The exact retry count is nondeterministic.

## Glossary

- **FIFO:** First in, first out.
- **SPSC:** Single producer, single consumer.
- **Atomic:** An operation that avoids data races on its own atomic object and can establish synchronization depending on memory order.
- **Acquire/release:** A synchronization pair that orders earlier writes before later reads across threads.
- **Cache line:** Unit of cache transfer/coherence on common CPU architectures.
- **Backpressure:** A policy for slowing or rejecting producers when downstream processing cannot keep up.
- **Latency percentile:** A summary of a distribution, not a guaranteed maximum.

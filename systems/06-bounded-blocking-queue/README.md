# Bounded blocking queue with graceful worker shutdown (C++20)

## A practical systems problem

Imagine a support platform receiving 8,000 incoming events from several API threads. It has **three background workers** that parse the events and store results.

- If producers outpace consumers and the queue is **unbounded**, memory can grow without limit.
- If producers and consumers access the same container without synchronization, data races corrupt the results.
- If the application shuts down while workers are waiting, threads can hang forever.

This C++20 lesson implements a **fixed-capacity, mutex-protected FIFO queue** for multiple producers and multiple consumers. When full, `push()` blocks (**backpressure**). When empty, `pop()` waits. `close()` wakes every waiting thread; consumers may drain remaining entries, then stop when `pop()` returns `std::nullopt`.

This is a simple **production-style design exercise**, not a low-latency lock-free queue.

## Small example

The demo creates a queue with capacity 3, starts one producer sending five synthetic event IDs, and two consumer workers. All workers stop automatically after the producer closes the queue.

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic -pthread main.cpp -o queue_demo
./queue_demo
```

Expected line:

```text
All 5 synthetic events dispatched and workers stopped.
```

## The API

```cpp
systems::BoundedQueue<int> queue(10);

queue.push(123);       // Wait for space, or return false if closed.
auto event = queue.pop(); // Wait for data, or return nullopt if closed and empty.
queue.close();          // Wake blocked producers and consumers.
```

The real code should **check the boolean result** of `push()`: the queue might be closed. `try_push()` and `try_pop()` never wait for capacity or data. `size()`, `capacity()`, and `is_closed()` aid inspection, but observing the size does **not** reserve a slot. Do not write code that does `if (queue.size() < capacity) queue.try_push(x)` and expects atomicity.

The implementation accepts move-only types such as `std::unique_ptr<T>`. Since moving the queue's value into the return object can potentially throw, consumers should prefer `T` types with non-throwing moves if they need strong exception guarantees.

## Test concurrency and shutdown

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic -pthread tests.cpp -o queue_tests
./queue_tests
```

Tests check FIFO, bounded capacity, rejected sends after shutdown, draining existing entries, move-only data, waking a blocked reader and writer, and **4 producers × 2,000 unique IDs** consumed by three workers. The final test checks each ID was consumed **exactly once**.

Where available, an equivalent CMake/CTest setup is included:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## What is synchronized?

A `std::mutex` protects the deque, the current count and the closed flag. Two `std::condition_variable` instances wake threads when a new item appears or space becomes available. Each condition wait uses a **predicate** to handle spurious wakeups and state changes safely.

`close()` takes the same mutex as producers and consumers, so no new push can succeed *after* the close completes. Existing items are still visible to consumers, allowing graceful draining.

**Limitations:** Threads are not guaranteed fair scheduling; blocking operations may incur context switches; the implementation uses dynamically allocated deque storage and is unsuitable for hard real-time or lock-free requirements. Closing is final (no reopen), and the queue does not automatically shut down other resources.

## Exercises

1. Explain what happens if the queue capacity is 1 and producers publish faster than consumers.
2. Change the workload to 8 producers and 1 consumer. Does every event remain accounted for?
3. Add a `try_push()` call inside the demo and count immediate rejections.
4. Why is `close()` preferable to abruptly terminating worker threads?
5. For very low-latency market data, compare this blocking queue with a ring buffer and a single-producer/single-consumer queue. What performance trade-offs change?

**Key lesson:** A fast event pipeline is not only about throughput. It must also behave predictably when overloaded or shutting down.

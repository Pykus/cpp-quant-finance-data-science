# C++20 Single-Producer / Single-Consumer Event Queue

## A real problem: two threads need to exchange market events

Imagine a market-data receiver reading messages from a network socket. A
different thread applies those messages to an analytics engine or order book.
The receiver must preserve event order, avoid allocating memory for every
message, and avoid blocking on a mutex in its hot path.

A **single-producer, single-consumer (SPSC) queue** connects those threads:

    Network receiver (producer) -> bounded FIFO -> analytics worker (consumer)

This lesson implements that hand-off for small, trivially copyable market
events. The producer calls try_push; the consumer calls try_pop. Both calls
return immediately: success or false for a full/empty queue. There is **no
silent overwrite**. If the queue fills, the application must decide whether
to retry, slow the feed, count drops, or fail explicitly.

The queue is intentionally separate from the previous
[non-thread-safe ring buffer](../03-ring-buffer/). This lesson introduces
cross-thread publication and memory ordering.

## What the program actually does

An example event contains a sequence number, a price in integer ticks, a
quantity, and a side flag. These are fictional updates, not live market data.

1. Store three events in a queue with logical capacity three.
2. Attempt a fourth push: it fails because the queue is full.
3. Pop the three events in exactly the insertion order.
4. A separate test starts a **producer thread and a consumer thread** and
   transfers 100,000 synthetic messages, validating every received field.

The representative result:

~~~text
Queue full: event 4 must wait (backpressure).
Received #1 price_ticks=10025 quantity=2
Received #2 price_ticks=10050 quantity=4
Received #3 price_ticks=10075 quantity=6
All three queued events processed in FIFO order.
~~~

## Build, run and test

Requires a C++20 compiler (GCC, Clang or recent MSVC), standard library, and
threads. No dependencies, network access or live trading credentials needed.

From this directory on Linux:

~~~bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread main.cpp -o spsc_demo
./spsc_demo
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror -pthread tests.cpp -o spsc_tests
./spsc_tests
~~~

Alternatively (cross-platform):

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

The test executable prints the actual number of checks it passes. Its five
scenarios cover one-slot behavior, full/empty boundaries, wraparound at
non-power-of-two capacities, reference-FIFO agreement over thousands of
operations, and producer/consumer hand-off under contention.

For extra runtime checking on compilers with sanitizers:

~~~bash
g++ -std=c++20 -O1 -g -Wall -Wextra -Wpedantic -Werror -pthread -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o spsc_sanitized
./spsc_sanitized
~~~

AddressSanitizer/UndefinedBehaviorSanitizer do not prove the absence of data
races. ThreadSanitizer is a different optional tool and may not work on all
runtimes. No timing or throughput promise is made.

## The API

~~~cpp
#include "spsc_event_queue.hpp"

market_data::SpscEventQueue<MyTrivialEvent, 64> queue;

// Call only from the producer:
const bool accepted = queue.try_push(event);

// Call only from the consumer:
MyTrivialEvent received{};
const bool available = queue.try_pop(received);
~~~

Capacity is the number of *usable events* (not underlying array elements).
There is exactly one extra array slot to distinguish full from empty. The
template statically rejects zero capacity, non-trivially-copyable messages,
and message assignments that might throw.

**Key invariants:**

- Only the producer writes the producer index.
- Only the consumer writes the consumer index.
- The producer copies a message before publishing the new producer index
  with **release** semantics.
- The consumer reads that index with **acquire** semantics before reading
  the message; it returns a slot to the producer using another release/acquire
  pair.
- The indices are kept on separately aligned 64-byte boundaries to reduce
  cache-line contention on common hardware; that alignment is not a universal
  guarantee about cache design.

These rules make accessing a non-atomic message slot safe when the SPSC
contract is followed. The atomic indexes do not turn this into an
unrestricted multi-producer or multi-consumer data structure.

## What this does not do

- It cannot be shared by two producers or two consumers.
- Push/pop operations never wait; callers decide what to do on failure.
- It provides no built-in shutdown protocol or notification when data arrives.
- Messages occupy preallocated storage. For general objects with ownership,
  pointers or exceptions, different lifetime handling would be needed.
- A false push does not imply that a concurrent pop cannot have freed a slot
  meanwhile; it is a snapshot-based failure, not a global transaction.
- A false pop likewise means the consumer did not observe an available item.
- The algorithm's bounded steps do not guarantee underlying atomic operations
  are lock-free on every possible target architecture.
- The 64-byte alignment, workload and scheduling are illustrative, not
  performance measurements or evidence of production HFT suitability.

## Learn the concurrency reasoning

Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md). It explains the network
handoff story, full/empty detection, acquire/release ordering, thread ownership
and the comparison with a mutex in terms suitable for a first-year student.

Suggested next exercise: add a counter in the producer recording failed push
attempts, and explain why a push failure is not automatically a dropped packet.

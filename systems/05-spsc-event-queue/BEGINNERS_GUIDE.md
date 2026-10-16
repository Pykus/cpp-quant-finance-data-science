# Beginner's Guide: A Queue Between Two Threads

## 1. A concrete example

A trading-data service receives updates over a network. One thread parses
incoming messages; another calculates statistics or updates an order book.
The two jobs can run on different CPU cores.

Each update might carry:

| Field | Example | Meaning |
| --- | ---: | --- |
| sequence | 1 | The order in which an event was received |
| price_ticks | 10025 | Price represented as an integer number of ticks |
| quantity | 2 | Units reported for the event |
| side | 0 | A synthetic code for buy (1 would mean sell) |

The hand-off must not reorder updates. Losing an update accidentally could make
the analytics thread reconstruct an incorrect stream. The queue therefore
returns **false** rather than silently discarding or overwriting an event.

## 2. What is a FIFO?

FIFO stands for "first in, first out". Think of a queue at a shop: the first
person to join leaves first. In a software FIFO, a push adds an item to the
back, and a pop removes the oldest available item.

For a queue holding three items:

    Push 10 -> [10]
    Push 20 -> [10, 20]
    Push 30 -> [10, 20, 30] (full)
    Push 40 -> false; still [10, 20, 30]
    Pop     -> 10; now [20, 30]
    Push 40 -> [20, 30, 40]

We use a circular array to avoid shifting elements on each operation.

## 3. Why is an extra array slot useful?

A circular index eventually wraps from the last array position to zero.
With only two indices, an empty ring and a full ring might both have the same
read and write positions. We avoid this ambiguity by allocating one extra
internal element, while exposing only the requested logical capacity.

For example, a queue with capacity 3 uses an internal array of 4 slots:

- Empty: read index equals write index.
- Full: advancing the write index once would equal the read index.
- A successful push writes into the current slot and then advances write.
- A successful pop reads from the current slot and then advances read.

Index advancement takes a fixed number of operations. No heap allocation
occurs during push/pop for the supported simple message types.

## 4. Why ordinary variables are not enough

Suppose Thread A writes a message and Thread B reads it. If they access a
shared variable concurrently without synchronization, the program may have a
**data race**, which leads to undefined behavior in C++.

This queue uses two atomic indices: one owned by each thread. The event data
themselves remain ordinary values; the indices coordinate when a slot is
safe to touch.

The producer performs the following sequence:

    A1. Read its own write index (relaxed).
    A2. Read the consumer's index (acquire) to check for free space.
    A3. Copy the complete event into an unused array slot.
    A4. Publish the new write index (release).

The consumer performs the matching sequence:

    B1. Read its own read index (relaxed).
    B2. Read the producer's index (acquire) to check for a ready event.
    B3. Copy the event out of the published slot.
    B4. Publish the new read index (release).

**Release** says "everything I wrote before this publication must be visible
to another thread that acquires this publication." **Acquire** synchronizes
the other thread with that release. The consumer cannot read a slot before
observing its producer's release. The producer likewise cannot reuse a slot
before observing the consumer's release that returns it to the pool.

The precise memory-ordering rules matter. Replacing the acquire and release
operations with relaxed operations everywhere would break this guarantee.
Using a normal integer instead of an atomic index would also be incorrect.

## 5. Why the SPSC limitation matters

SPSC means **single producer, single consumer**.

Only one thread may push; only one (different or same) thread may pop.
The producer exclusively owns modifications to the write index, and the
consumer exclusively owns modifications to the read index.

With two producers, both could calculate the same write location and attempt
to fill the same slot, corrupting the stream. With two consumers, both could
read the same message. This queue deliberately does not implement the
compare-and-swap operations or per-slot state tracking used by more general
concurrent queue algorithms.

You may create many separate SPSC queues, but each individual instance has
exactly one producer role and one consumer role.

## 6. Why align the indices?

Modern CPUs access memory in cache lines. If two CPU cores repeatedly modify
different variables that occupy the same cache line, the line may bounce
between cores even though the variables are logically unrelated. This is
called **false sharing**.

The two atomic indices have 64-byte alignment to reduce this issue on common
processors. It is a reasonable teaching choice, not a guarantee: cache-line
size depends on hardware. Proving a performance benefit requires benchmarking
on the target system with representative messages and CPU placement.

## 7. What happens when it fills?

A full buffer is not automatically a bug. It can happen if the producer is
faster than the consumer. This is called **backpressure**.

A producer might:

- Retry later after doing other useful work.
- Yield the CPU while it is waiting.
- Record a failed enqueue attempt as a metric.
- Apply an explicit application-level drop or reconnect policy.

Which response is correct depends on the system. **A failed push is not a
silently lost event:** the caller knows the event was not accepted and must
decide its fate.

A real market-data application also needs a shutdown protocol, bounded
end-to-end latency, and a plan for overload; those are outside this lesson.

## 8. Follow the sample

Compile and run the demo using the exact commands in README.md. Three messages
go into a capacity-three queue. The fourth is rejected. Three pops then yield
the original sequence numbers 1, 2 and 3.

The demo runs these steps in one thread for clarity. **The actual thread
safety is exercised by the concurrent test** in tests.cpp: two threads transfer
100,000 messages and independently verify ordering, sequence, price, quantity
and side fields.

Additional tests fill/drain the ring repeatedly, exercise capacity one, and
compare thousands of operations against the standard library's deque.

## 9. Is this "lock-free"?

The algorithm does not use a mutex and each try operation executes a bounded
number of steps. But C++ does not guarantee that every atomic size type maps
to lock-free machine instructions on every architecture. Additionally, a
complete application can still block or spin while waiting for a queue item.
It would be misleading to claim that the entire network-processing service
has guaranteed real-time latency based on this small class.

## 10. Exercises

1. Draw the internal four positions used by a capacity-three queue after
   pushing three events, popping one, and pushing another.
2. Why does the consumer check the producer's index using acquire rather
   than relaxed ordering?
3. What happens if a second thread also calls try_push on the same queue?
4. Why should a caller check the returned boolean from try_push?
5. How would you measure messages per second and the 99th-percentile time
   spent waiting when the queue is full? Which other measurements would be
   needed before comparing this queue with a mutex-protected implementation?

### Check your understanding

1. The older event is popped first; the new event reuses the freed slot.
2. Acquire is needed to see the complete event written before the matching
   release on the producer's index.
3. Two producers violate the ownership assumption and may write the same
   slot concurrently.
4. A failed push means the event remains with the caller; silently ignoring
   the result could lose data at the application level.
5. Record production timestamps, consumption timestamps and failed push
   attempts with a carefully calibrated benchmark, isolate logging overhead,
   consider core pinning and load variation, and report latency
   distributions including p50, p99 and p99.9. This lesson does not claim
   measured latency.

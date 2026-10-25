# Lesson 09 — Detect gaps in UDP market-data sequence numbers

An order-book feed may attach a monotonically increasing 32-bit sequence
number to each datagram. UDP can drop packets or deliver them out of order.
Learn to distinguish normal progress, skipped values, and late/duplicate
arrivals with a **constant-memory C++20 tracker**.

## Contract

Call `SequenceTracker::observe(sequence)` for each packet number in arrival
order. The function returns:

- `First`: the first sequence observed sets a starting point.
- `InOrder`: exactly the next sequence arrived.
- `Gap`: the sequence jumped forward; `missing` is how many sequence
  numbers were skipped.
- `LateOrDuplicate`: a previously passed or ambiguous sequence arrived;
  the expected sequence is not changed.

Example with first observed sequence 42:

```text
42 -> First            next=43
43 -> InOrder          next=44
46 -> Gap (missing=2)  next=47
45 -> LateOrDuplicate  next=47
47 -> InOrder          next=48
```

Because sequence numbers are `std::uint32_t`, unsigned subtraction
automatically wraps modulo 2^32. We interpret only positive forward jumps
smaller than 2^31 as gaps; differences at or above 2^31 are ambiguous and
classified as late. The tracker cannot detect a missing packet **before**
it has observed a later packet.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Alternative without CMake:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o gap_tests
./gap_tests
```

Tests cover first packet, normal progression, missing values, late
packets, duplicate detection, reset, modular wraparound, the half-range
ambiguity and 200,000 contiguous synthetic messages.

## Engineering notes

Each observation takes O(1) time and O(1) memory. No allocation, I/O
or network sockets are required in this standalone module. The tracker
has no locks and must be mutated by one thread at a time.

This is **not** a production multicast market-data recovery engine: it
does not buffer reordered packets, request retransmissions, authenticate
sources, or handle sequence epochs. A real feed protocol defines its
own wrap rules and recovery sequence.

Continue with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) and try counting
the total number of skipped messages without double-counting late arrivals.

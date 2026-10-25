# Beginner's guide — from UDP messages to gap detection

## Why a sequence number?

Imagine a market-data feed sends five fictional updates numbered
10, 11, 12, 13, 14. A receiver sees 10, 11, 13, 14.
The missing number 12 is evidence that at least one message may have
been lost or delayed. Without a sequence number, this is harder to detect.

## Track the next expected number

After the first packet (say 10), expect 11. If 11 arrives, expect 12.
If 13 arrives instead, report a gap of one and expect 14. If 12
comes later, label it late and do not roll the state backward.

The state is only a flag and one 32-bit number. That is why memory use
does not grow with the number of packets.

## Why unsigned arithmetic matters

The 32-bit maximum is 4,294,967,295. After that, 0 is the next value.
In C++, subtraction on unsigned 32-bit integers has well-defined
wraparound. For example, when expecting 0, observing 2 gives a
difference of 2, signaling that 0 and 1 were not seen.

But modular arithmetic has a limit: if the difference is at least
2,147,483,648, we cannot reliably distinguish a huge forward jump
from an old packet. The implementation conservatively chooses the
late/ambiguous classification. A real protocol needs defined epochs.

## Read the C++ code

- `std::uint32_t` communicates that a sequence has exactly 32 bits.
- `enum class EventKind` prevents confusing different event categories.
- `Observation` returns the classification, missing count, and next value
  in one result.
- `[[nodiscard]]` warns when callers discard a detected gap.
- `noexcept` is appropriate because observation uses only simple arithmetic
  and assigns fields, with no allocation or exceptions.
- `reset` begins a new session after the caller explicitly recognizes
  a fresh feed epoch.

## A practical limitation

A UDP receiver typically needs a gap-recovery protocol, packet buffers,
snapshots and checksums. This first exercise deliberately does **not**
claim to implement any of them.

## Your exercise

Extend the result to include a continuously updated count of observed
in-order, gap, and late packets. Write a wraparound regression test and
explain why an ambiguous distance must not move the expected sequence.

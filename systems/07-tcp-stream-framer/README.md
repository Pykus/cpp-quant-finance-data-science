# C++20 Systems: Bounded TCP Stream Framing

## The actual problem

You write a market-data service or any TCP client. The remote side sends two
messages:

- **ADD|7|10123|250** — fictional order 7, price 10123 ticks, quantity 250.
- **CANCEL|7** — fictional cancellation of order 7.

Your program calls recv() and obtains **bytes**, not "one message per call".
It might receive half of the first message, both messages together, or any
other combination. A parser that assumes each read contains exactly one full
message will sometimes lose data or corrupt a message.

This lesson implements a **small, deterministic framing layer** that joins
arbitrary chunks into complete payloads. It has no socket dependency, so the
hard part can be tested offline.

## Wire format

Every frame has exactly two header bytes followed by 1 to MaxPayload bytes:

| Bytes | Meaning |
| --- | --- |
| 0–1 | Unsigned, **big-endian** payload length, **excluding** this header |
| 2 onward | Exactly that number of uninterpreted payload bytes |

The bytes **00 03 41 42 43** mean a 3-byte payload, **ABC**. The decoder
does not know whether the payload represents an order, a JSON record, or
anything else.

**Why a length header?** TCP is a byte stream. Without an agreed delimiter
or length, the receiver cannot decide where one message ends and the next
begins. This is one illustrative protocol, not an exchange standard.

**Why a bound?** If a corrupt header advertises an enormous message, blindly
allocating that size can exhaust memory. Here a compile-time MaxPayload limits
storage and accepted lengths. The default is 64 bytes; configure a different
bound for your own protocol.

## Run the working example

Requires a C++20 compiler (for std::span and std::byte). On Linux:

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o stream_demo
./stream_demo
~~~

Output:

~~~text
Event 1: ADD|7|10123|250
Event 2: CANCEL|7
Decoded 2 events
~~~

The demo builds a wire stream containing **both** framed messages, splits
it into deliberately uneven chunks, and passes every chunk to the decoder.
The output lines are assembled **only after a complete payload arrives**.

## Tests

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o stream_tests
./stream_tests
~~~

The test program reports its actual number of successful checks. It covers:

- Rejecting empty and oversized outgoing payloads.
- Exact big-endian header bytes and binary payloads (including 0x00).
- Every two-chunk split of a representative frame.
- Reading exactly one byte at a time.
- Multiple complete frames in one input chunk.
- A trailing incomplete frame finished by a later chunk.
- Rejecting zero and oversized advertised lengths **before** reading payload.
- Sticky error state and explicit reset() after malformed input.
- Successful frames followed by an invalid header in the same chunk.
- Prefix lengths greater than 255 (a 300-byte payload with a 512-byte limit).
- Callback exceptions, buffer reset, and recovery.
- **512 synthetic frames**, delivered across deterministic irregular chunks.

Optional sanitizer check on a supported compiler:

~~~sh
g++ -std=c++20 -g -O1 -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o stream_tests_san
./stream_tests_san
~~~

Or use CMake:

~~~sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

## Using the library

**framing.hpp** is header-only. Both encoder and decoder use bounded
std::array storage, not per-frame heap allocation:

~~~cpp
#include "framing.hpp"

stream_framing::StreamDecoder<64> decoder;
auto report = decoder.feed(received_bytes, [](std::span<const std::byte> payload) {
    // Process or copy the complete message here.
});
if (report.status != stream_framing::FeedStatus::ok) {
    // Discard this connection or reset according to your protocol policy.
}
~~~

The variable received_bytes must be an actual std::span<const std::byte>
over bytes obtained from a socket or other transport. This snippet is an
integration outline; **main.cpp** is the complete runnable program.

**FeedResult** tells you how many bytes were consumed and complete frames
were emitted. On an invalid header, it stops at that header, reports an
error, and **refuses subsequent data until reset()**. This prevents
accidental interpretation of corrupted streams as valid messages.

**Important ownership rule:** the callback's payload view points into
the decoder's internal fixed buffer. Use it inside the callback or copy it
if you need to retain it. It may be overwritten by the next frame.
Do not call feed() or reset() **inside** the callback, and do not use the
same decoder simultaneously from multiple threads.

## Scope and limitations

This lesson solves **message boundary recovery**, not network I/O or market
matching. **main.cpp** simulates receive chunks; it does **not** open a TCP
socket or benchmark latency. The text payloads are synthetic events,
not real exchange packets.

The parser accepts arbitrary bytes as payload and performs no application
validation, message authentication, checksums, sequence checks, or timeouts.
A disconnected stream with an incomplete frame remains incomplete; the
caller must check buffered_bytes() when the connection closes. No automatic
resynchronization is attempted after an invalid length — in most real
protocols, closing the connection is safer than guessing a new boundary.

Encoding fails instead of truncating oversized payloads. A handler exception
propagates; because a frame is reset before callback invocation, the parser
can be reused, but unprocessed bytes from that **same** feed() call are not
automatically replayed. Production callers must define their own error policy.

Next steps for a real low-latency feed receiver: implement a proper socket
loop, message-type validation, backpressure, reconnect policy, timestamps,
end-to-end benchmarks, and fuzz testing.

## Learn from the beginning

Open [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for a byte-by-byte explanation,
a worked fragmentation example, and exercises.

Reference material:
- [cppreference: std::span](https://en.cppreference.com/w/cpp/container/span)
- [cppreference: std::byte](https://en.cppreference.com/w/cpp/types/byte)
- [POSIX recv()](https://pubs.opengroup.org/onlinepubs/9699919799/functions/recv.html)

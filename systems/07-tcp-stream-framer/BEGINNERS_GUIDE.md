# First steps: why TCP needs a framing parser

You do not need networking experience to understand this project. The entire
program can be run on your laptop without opening a network connection.

## 1. What is a message?

Imagine a trading application sends two text commands:

- PING
- OK

Your application needs to know that these are **two separate messages**.
But a TCP connection delivers a stream of bytes. Think of a long conveyor
belt carrying letters without package boundaries.

Your receiver might read:

- P
- INGOK

Or it might read:

- PINGOK

Neither outcome is a bug. A TCP read boundary does not represent a message
boundary. The sender's calls to send() do not force matching recv() calls.

**Real-world use:** a market-data receiver, telemetry gateway, chat server,
or any service accepting many messages on one connection must reconstruct
message boundaries before interpreting individual records.

## 2. Add a length before every payload

We invent a tiny wire protocol. Every message begins with two bytes holding
the size of the payload. The number does **not** include the two-byte prefix.

Using hexadecimal (base 16), the two messages become:

~~~text
00 04 | 50 49 4E 47 | 00 02 | 4F 4B
length  P  I  N  G    length  O  K
~~~

ASCII letters have numerical byte codes: P is hex 50, I is hex 49, N is
hex 4E, G is hex 47, O is hex 4F, and K is hex 4B.

- **00 04** = 4 bytes in the first payload.
- **00 02** = 2 bytes in the second payload.
- Total stream length = (2 + 4) + (2 + 2) = **10 bytes**.

The length is stored with the **most significant byte first** (big-endian).
The two-byte number **01 2C** equals 256 + 44 = 300, not 44.

## 3. Feed the parser in the worst possible chunks

Consider the same 10-byte stream split into three reads:

~~~text
Read 1: 00
Read 2: 04 50
Read 3: 49 4E 47 00 02 4F 4B
~~~

After read 1, the decoder has only one header byte. It cannot know how
much payload it needs.

After read 2, the header is complete: the payload must contain four
bytes. Only one payload byte, P, has arrived.

During read 3, bytes I, N, and G finish the first payload. **Now**, and
not before, the decoder invokes its message callback with PING.
It then immediately starts a new frame, reads the prefix 00 02 and
invokes the callback again with OK.

This is why the parser is **stateful**: it remembers partial progress
between calls instead of starting from scratch with every recv().

## 4. Which C++20 tools solve the problem?

**std::byte:** represents raw data without pretending that the data is
already a character or a number. Converting to a numerical byte uses
std::to_integer.

**std::span<const std::byte>:** a read-only view of an existing block of
bytes. It does not own or copy the bytes. A span can describe a complete
network chunk or a complete decoded payload.

**std::array<std::byte, MaxPayload>:** fixed-capacity storage for the
incomplete payload. A known maximum size helps avoid unpredictable
heap allocations in a performance-sensitive loop.

**Template parameter MaxPayload:** a number chosen at compile time, such
as 64. Different protocol instances can use different limits without
making them run-time allocations.

**Callback:** a function supplied by the caller. The parser calls it
whenever exactly one complete payload has been reconstructed. The parser
does not interpret the contents.

**[[nodiscard]]:** reminds callers to check whether a feed failed or
an encoder rejected a too-large message.

## 5. Why we reject certain frames

An attacker or broken sender could announce:

~~~text
FF FF
~~~

That two-byte prefix means 65,535 payload bytes. If the application expects
messages of at most 64 bytes, accepting that number would be dangerous.

The decoder checks the advertised length as soon as the second prefix byte
arrives. It rejects **0** and anything larger than MaxPayload.

After a bad header, the decoder stays in an error state. Its caller must
close the connection or **explicitly call reset()**. Silently trying to guess
where a new valid header starts could accept corrupted data.

A valid but incomplete frame is different from an invalid one: the decoder
holds it until more bytes arrive, or until the caller closes the connection.
It cannot deduce that the other side has disappeared by itself.

## 6. Why the callback's data cannot be kept forever

The decoder reuses one internal fixed-size array. When a complete message
arrives, the callback receives a span pointing into that array.

If you need the payload later, **copy its bytes while inside the callback**.
Do not store the span itself: a later feed may overwrite the same memory.

This matters in low-latency designs because avoiding copies can save work,
but it also makes ownership and lifetime rules critical.

## 7. How to learn from the implementation

Read the files in this order:

1. **main.cpp** — two fictional trading messages and simulated small reads.
2. **framing.hpp** — length encoder, three pieces of parser state, callback.
3. **tests.cpp** — every two-part split, malformed headers and long streams.
4. **README.md** — the design contract, build commands and limitations.

Build the demo before changing the code. Then experiment with different
chunk sizes: the final two decoded messages should stay the same.

## 8. Exercises

**Exercise A — calculate the payload boundary.** A header is 00 05 and the
following bytes are 41 42 43. Should the callback run yet? How many more
bytes are needed?

**Exercise B — split a stream.** Take the ten bytes for PING and OK.
Invent four different ways of dividing them into recv() chunks. Confirm the
decoder emits exactly two messages each time.

**Exercise C — overflow protection.** A decoder with MaxPayload = 64 sees
00 41. How many bytes must be present before it can reject the frame?
Will it consume the following payload byte?

**Exercise D — ownership.** What happens if a callback saves a span into
the decoder's internal buffer and another frame arrives later?

**Exercise E — production design.** List at least three things missing
before this can receive real exchange market data. Think about socket
handling, protocol-specific validation, authentication, and performance
measurement.

### Hints

- A prefix is not part of the payload size.
- A message must be complete before emission.
- A two-byte prefix is enough to reject impossible lengths.
- A span is not an owning container.
- Successful unit tests do not replace real network tests or benchmarks.

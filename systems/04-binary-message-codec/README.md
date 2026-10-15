# C++20: Safe Binary Message Encoding and Decoding

## Why this matters

Market feeds, sensors and network services often exchange *binary messages*: compact sequences of bytes instead of JSON. A careless decoder can read beyond a packet or misinterpret an integer. This lesson builds a fixed-size, versioned wire protocol using `std::span`, integer arithmetic and `std::variant`, with two fictional events: **Add** (order ID, price and quantity) and **Cancel** (order ID). This is educational code, not an exchange protocol or matching engine.

## Prerequisites and terminology

- A **byte** contains eight bits. A **wire format** specifies the meaning and location of each field.
- **Big-endian/network byte order** means the most significant byte of a multi-byte integer comes first. Hexadecimal `00 10` is decimal 16.
- A **tick** is a discrete price increment. At a hypothetical $0.01 per tick, 10123 ticks corresponds to $101.23. Integer ticks avoid many floating-point rounding surprises.
- `std::span<const std::byte>` is a read-only, non-owning view of contiguous bytes **with a length**. The underlying storage must remain alive during decoding.
- `std::optional<T>` either holds a value or is empty; `std::variant<Add, Cancel>` holds one of two message types.

## Packet layout

One packet contains exactly one event. Its first four bytes form a header. All multi-byte numbers are unsigned and big-endian.

| Offset | Bytes | Meaning |
| --- | ---: | --- |
| 0 | 1 | Protocol version: 1 |
| 1 | 1 | Message kind: 1 Add, 2 Cancel |
| 2–3 | 2 | Payload size: 16 Add, 8 Cancel |
| 4–11 | 8 | Nonzero order ID |
| 12–15 | 4 | Nonzero price in ticks (Add only) |
| 16–19 | 4 | Nonzero quantity (Add only) |

An Add packet is **20 bytes** (4-byte header + 16-byte payload); a Cancel packet is **12 bytes** (4 + 8).

### Worked example

Add order ID 7, price 10123 ticks and quantity 250:

```text
01 01 00 10 | 00 00 00 00 00 00 00 07 | 00 00 27 8B | 00 00 00 FA
   header   |           ID            |    price    |   quantity
```

The header's `0x0010` means **16 payload bytes**, so the total packet is **4 + 16 = 20 bytes**. The price `0x278B` is decimal **10123**; the quantity `0xFA` is decimal **250**. At a one-cent tick, the example price represents **$101.23**.

## Decoder safety rules

1. Check at least four bytes exist **before reading** the header.
2. Reject unsupported versions and unknown message kinds.
3. Require declared payload size to match the kind's known fixed size.
4. Compare actual packet length with the expected length before accessing fields; reject both truncation and trailing bytes.
5. Reject zero order IDs, and for Add also reject zero prices and quantities.

The decoder returns an empty message and a specific `Error` on failure. The encoder refuses invalid messages by returning `std::nullopt`. Both use fixed-size arrays and perform no dynamic allocation.

## Build and test

C++20 is required for `std::span` and defaulted comparisons.

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o message_demo
./message_demo
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o message_tests
./message_tests
```

Expected demonstration output:

```text
frame bytes: 20
order ID: 7
price ticks: 10123
quantity: 250
```

The tests verify exact network-order bytes, both roundtrips, every incomplete packet prefix, unsupported versions, unknown kinds, length mismatches, extra bytes and invalid fields. A failure returns a nonzero process exit code; no tests rely on disabled `assert` macros.

With a compiler that supports sanitizers, additionally run:

```sh
g++ -std=c++20 -g -O1 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o message_tests_sanitized
./message_tests_sanitized
```

## Design boundaries and pitfalls

**This parses one complete packet, not a TCP stream.** TCP can split or combine messages; a separate bounded framing layer must accumulate full packets before decoding. Never allocate a buffer blindly from an untrusted length field. `std::span` does not own memory. This example omits checksums, authentication, concurrency, timestamps, replay protection and exchange semantics.

## Exercises (with solutions)

1. **Cancel size?** Four header bytes plus eight ID bytes = **12 bytes**.
2. **Does `00 10` include the header?** No: it means **16 payload bytes**, not 16 total bytes.
3. **TCP supplies a 20-byte message in chunks of 9 and 11 bytes. What now?** Accumulate the fragments in a bounded buffer; decode only after the entire 20-byte packet is available.
4. **Why reject trailing bytes?** They can indicate concatenated packets, broken framing, or malformed input; silently ignoring them hides protocol errors.
5. **Why integers for prices?** Tick counts are exact discrete values; the correct price requires knowing the instrument's tick size.

## Further reading

- [`std::span`](https://en.cppreference.com/w/cpp/container/span)
- [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant)
- [C++ Core Guidelines: bounds safety](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-bounds)

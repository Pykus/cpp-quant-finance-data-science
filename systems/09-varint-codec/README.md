# Lesson 09 — A bounded unsigned varint codec for event feeds

## The practical problem

A telemetry gateway, exchange market-data feed, or distributed log passes many small nonnegative numbers: an event ID of 127, instrument ID of 300, or record count of 15,000. Always using 8 bytes for each integer is straightforward, but it spends bandwidth on mostly zero bytes.

An **unsigned base-128 variable-length integer** (varint) instead encodes seven payload bits in each byte. The high bit tells the receiver whether more bytes follow. Small integers take fewer bytes:

| Decimal value | Bytes in hex | Number of bytes |
| ---: | --- | ---: |
| 127 | 7F | 1 |
| 300 | AC 02 | 2 |
| 15000 | 98 75 | 2 |

This C++20 library **encodes unsigned 64-bit integers into a caller-owned buffer and decodes them safely**, including when bytes arrive one by one through a fragmented network stream. No heap allocation is performed by the codec's successful encode/decode path.

## Compile and run

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o varint_example
./varint_example
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic tests.cpp -o varint_tests
./varint_tests
~~~

Or use the included optional CMake/CTest file.

### Important guarantees

- Every uint64 value encodes in **1–10 bytes**.
- Encoding checks the output span's capacity **before writing anything**. A too-small buffer throws std::length_error and remains unchanged.
- The decoder has explicit states: **need_more**, **complete**, **invalid**.
- It rejects **noncanonical overlong encodings** (such as 80 00 for zero) and values overflowing 64 bits (such as a tenth byte larger than 01).
- It does not read outside the provided span. Decoding stops at the end of the first complete integer and returns the **number of bytes consumed**, so the caller can find the next message.
- The incremental Decoder retains state across fragments and requires reset() between values.

The test binary checks known encodings, incomplete data, invalid tenth bytes, no partial writes, concatenated values and **100,000 reproducible randomized round trips**. Run with address/undefined sanitizers where available:

~~~sh
g++ -std=c++20 -O1 -g -Wall -Wextra -Werror -pedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o varint_sanitized
./varint_sanitized
~~~

**Limitations:** Varints encode unsigned integers, not full packets. They do not supply authentication, checksums, message lengths, integer schema tags, congestion control, or guarantees about endian handling for other fields. This is a reusable building block, not a network protocol. Low-latency performance claims require benchmarks on real target hardware.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for a binary walkthrough, design choices, and exercises.

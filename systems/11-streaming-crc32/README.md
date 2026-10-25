# Lesson 11 — Streaming CRC-32 for data integrity checks

Learn how a byte-oriented CRC can detect accidental corruption in
messages or files. This C++20 exercise implements **CRC-32/ISO-HDLC**
(the common reflected "IEEE CRC-32"), with incremental updates.

## The contract

- Polynomial (reflected): `0xEDB88320`.
- Initial register: `0xFFFFFFFF`.
- Final XOR: `0xFFFFFFFF`.
- Input and result are byte-oriented. Text is interpreted as raw bytes.
- Updating in chunks must produce the same result as processing all
  the bytes at once.

Known reference vector:

```text
"123456789" => 0xCBF43926
```

`Crc32::update(std::span<const std::byte>)` accepts binary spans,
while `update_text(std::string_view)` offers a convenient text-byte
interface. `value()` returns the current finalized checksum.

## Implementation

For each input byte, XOR it into the low bits of the running register.
Repeat eight times: shift right one bit, and when the old low bit was
1, XOR the reversed generator polynomial into the register.

The implementation is intentionally **table-free** and does not allocate
memory. Each update takes O(n) time and O(1) auxiliary space. A 256-entry
lookup table would reduce operations per byte but complicate a first lesson.

## Test and build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o crc_tests
./crc_tests
```

Tests compare known reference vectors, binary bytes vs text, reset,
and 10,001 deterministic split-input checks.

**Important:** CRC-32 is **not** a cryptographic hash, authentication
mechanism or digital signature; an attacker can deliberately modify data
and its CRC. Use authenticated integrity mechanisms when security matters.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md). **Exercise:** implement
the lookup-table optimization and benchmark it against the baseline.

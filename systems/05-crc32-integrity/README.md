# C++20 Systems Lesson 05: Detecting Packet Corruption with CRC-32

## Why this matters

In the [previous binary message codec lesson](../04-binary-message-codec/), we learned how to encode and decode a bounded packet without reading outside the supplied bytes. However, data can be accidentally modified while stored or transmitted. A damaged packet may still have a valid-looking message type, length, and quantity.

This lesson introduces **CRC-32/ISO-HDLC**, a 32-bit **checksum** calculated from the bytes of a message. We add the checksum to the end of the packet, then verify it on receipt. The example builds directly on the previous lesson's Add and Cancel events and demonstrates a message with one altered bit.

New to networking, bit operations, or C++20? Start with the thorough [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md), including a hand-worked calculation and a line-by-line explanation of the update loop.

## Terms and learning goals

- A **bit** is 0 or 1; a **byte** holds eight bits.
- A **payload** is the original message; a **trailer** contains additional data appended after it.
- A **checksum** is a short value calculated from a larger data block. It can detect accidental corruption, but collisions are possible.
- **CRC** stands for cyclic redundancy check, a family of algorithms that detect many patterns of accidental bit errors.
- **XOR** (the ^ operator) produces 1 when two bits differ.
- A **span** is a view of existing contiguous memory and its size; it does not own the bytes.
- **Big-endian** or **network byte order** stores the highest-order byte first.
- A **streaming** CRC processes successive chunks, preserving its internal state.

After completing the lesson, you should be able to calculate CRC-32 for known inputs, explain why updating in chunks works, append a fixed four-byte trailer, and validate it safely.

## Packet format

    [1 to 20 payload bytes] [4-byte big-endian CRC-32 trailer]

The bound of 20 bytes matches the previous lesson's Add packet. The protected packet needs at most 24 bytes and is stored in a fixed-size std::array. A Cancel packet is 12 + 4 = **16 bytes**; an Add packet is 20 + 4 = **24 bytes**.

### Worked numerical example

The ASCII bytes of 123456789 have a **known standard CRC-32 value of 0xCBF43926**. In big-endian order, the checksum trailer is CB F4 39 26. The transmitted data therefore contains **9 + 4 = 13 bytes**.

The calculation begins with the all-one 32-bit integer 0xFFFFFFFF. For ASCII A (0x41), the first XOR produces 0xFFFFFFBE. The next two bit-level rounds yield 0x7FFFFFDF and 0xD2477CCF. After all eight rounds and the final XOR, the reference checksum for A is **0xD3D99E8B**. See the guide for each operation.

## Source files

- **crc32.hpp**: allocation-free, incremental 32-bit CRC calculator; bounded append_crc and verify_crc helpers.
- **main.cpp**: encode an event using the previous lesson; append CRC, verify it, then flip one bit and show that verification fails.
- **tests.cpp**: independent standard reference vectors, streaming equivalence, boundary checks, corruption detection, and integration with both earlier message types.
- **CMakeLists.txt**: C++20 build, warnings as errors, and CTest registration.
- **BEGINNERS_GUIDE.md**: tutorial aimed at a first-year student.

## Build and run

From this lesson's directory within the repository, with the previous lesson present as a sibling directory:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
    cmake --build build
    ./build/crc32_demo
    ctest --test-dir build --output-on-failure

Or build directly with GCC:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o crc32_demo
    ./crc32_demo
    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o crc32_tests
    ./crc32_tests

The demonstration output is:

    Payload bytes: 20
    Transmitted bytes: 24
    CRC-32: 0x4F977B28
    Verified original event: yes
    Verified altered event: no

Tests signal failure by returning a nonzero process exit code. They do not depend on assert macros that might be disabled in Release builds. On Windows, run the executable in the location produced by your CMake generator.

## Program design and limitations

The implementation uses **std::uint32_t** so CRC arithmetic has a defined 32-bit width. Each of eight rounds per byte shifts the state and conditionally XORs the reflected polynomial **0xEDB88320**. It starts and ends with XOR against **0xFFFFFFFF**. The result conforms to **CRC-32/ISO-HDLC**, not an unspecified generic CRC-32.

The calculator preserves its state between update calls. The method value() can be called at any time without preventing further updates. The reset() method starts a fresh calculation. The output format is explicit rather than relying on a host CPU's native byte order.

**Important:** CRC is **not cryptographic authentication, encryption, or proof of sender identity**. An attacker can calculate a new CRC after tampering; two different messages can collide. A correct CRC also does not prove that a packet has valid protocol fields. Always validate the inner message separately with wire::decode, and use appropriate authentication for hostile inputs.

This code verifies **one complete bounded frame**. It does not reassemble TCP byte streams, authenticate data, or benchmark performance. The bitwise CRC favors clarity over throughput. No latency claim is made.

## Exercises

1. Compute the total size of a protected Cancel event. **Answer: 12 + 4 = 16 bytes.**
2. Change one bit in the price bytes without updating the trailer. Check that verification fails.
3. Split the nine ASCII bytes into three chunks. Does the result differ from one update? **No.**
4. Why must the receiver call wire::decode after a successful CRC? **A CRC only checks accidental changes, not field validity.**
5. Explain why a matching checksum is evidence of integrity, but not a mathematical guarantee that the data is unchanged.

## Further reading

- [CRC-32/ISO-HDLC check vector and parameters](https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.crc-32-iso-hdlc)
- [std::span](https://en.cppreference.com/w/cpp/container/span)
- [std::byte](https://en.cppreference.com/w/cpp/types/byte)
- [C++ Core Guidelines: bounds safety](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-bounds)

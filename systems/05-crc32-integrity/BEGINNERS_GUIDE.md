# Beginner's Guide: CRC-32 Packet Integrity

You can follow this guide with no previous networking experience. Start with the short numerical examples; then compile the accompanying program and run its tests.

## 1. Why do we need an integrity check?

Suppose a sensor, a file, or a computer network carries a value such as 25. A computer stores data using **bits**, each equal to 0 or 1. A **byte** is eight bits. A noisy connection or damaged storage can accidentally change a bit. The resulting number may still look valid even though it is wrong.

A **packet** is a group of bytes passed together. Its **payload** contains the original information. A **checksum** is a number calculated from that payload. A **trailer** is extra information appended at the end. Our sender builds:

    [original payload bytes] [four checksum bytes]

The receiver calculates the checksum again and compares it with the supplied four bytes. If they differ, the data was accidentally altered or the checksum itself changed. **A matching checksum does not guarantee correctness:** two different messages can sometimes have the same checksum.

## 2. Understand the binary operations first

The symbol **^** means bitwise **XOR**, which produces 1 when the input bits differ and 0 when they are the same.

| Bit A | Bit B | A XOR B |
| --- | --- | --- |
| 0 | 0 | 0 |
| 0 | 1 | 1 |
| 1 | 0 | 1 |
| 1 | 1 | 0 |

For example, binary 1010 XOR 1100 is 0110.

The symbol **>> 1** shifts the bits of an unsigned number right one position. For example, 10110010 becomes 01011001. The lowest bit is removed and a zero enters at the left. An **unsigned integer** represents only nonnegative numbers. Our algorithm uses it so shifts and bit operations have defined behavior.

**Hexadecimal** uses base 16, with digits 0 through 9 and A through F. Each hexadecimal digit represents four bits. The notation 0xFF means hexadecimal FF, binary 11111111, and decimal 255.

## 3. What exactly is CRC-32?

**CRC** stands for cyclic redundancy check: a family of checksums based on binary arithmetic. CRC-32 produces a 32-bit result, or four bytes. There are several incompatible CRC-32 variants, so this lesson explicitly uses **CRC-32/ISO-HDLC**.

| Parameter | Value | Purpose |
| --- | --- | --- |
| Checksum size | 32 bits | Four extra bytes |
| Initial value | 0xFFFFFFFF | Initial internal state |
| Reflected polynomial | 0xEDB88320 | Feedback pattern in each round |
| Final XOR | 0xFFFFFFFF | Final transformation |
| ASCII test input | 123456789 | Standard reference vector |
| Expected CRC | 0xCBF43926 | Published check value |

A **polynomial** here is the fixed rule that defines this CRC variant. The *reflected* hexadecimal representation 0xEDB88320 is used by our right-shifting implementation. You do not need polynomial algebra to work through the code: first see how its fixed bit pattern is applied.

## 4. A numerical calculation with just one byte

The ASCII character **A** is the byte 0x41. **ASCII** is an encoding assigning numeric byte values to characters. CRC-32 starts from 0xFFFFFFFF. XOR the new byte into the low eight bits:

    0xFFFFFFFF XOR 0x00000041 = 0xFFFFFFBE

Next, process eight rounds for the eight bits of that byte.

**Round 1:** The rightmost bit of 0xFFFFFFBE is 0, so shift right without applying the polynomial:

    0xFFFFFFBE >> 1 = 0x7FFFFFDF

**Round 2:** The rightmost bit of 0x7FFFFFDF is 1. Shift right, then XOR the polynomial:

    0x7FFFFFDF >> 1 = 0x3FFFFFEF
    0x3FFFFFEF XOR 0xEDB88320 = 0xD2477CCF

There are six more rounds for this byte. After the eighth round, the final XOR with 0xFFFFFFFF produces **0xD3D99E8B**, the standard CRC-32 of ASCII A. The unit tests check this value, so errors in bit order or polynomial are caught.

You are not expected to calculate every round by hand. Understand why the result is deterministic: the same ordered bytes always cause the same operations.

## 5. Read the implementation line by line

Open crc32.hpp. The essential algorithm is:

    for (const auto byte : input) {
        remainder_ ^= std::to_integer<std::uint8_t>(byte);
        for (unsigned bit = 0; bit < 8; ++bit) {
            const bool low_bit = (remainder_ & 1u) != 0u;
            remainder_ >>= 1u;
            if (low_bit) {
                remainder_ ^= kReflectedPolynomial;
            }
        }
    }

Read each line:

1. The outer **for loop** visits each input byte in order. The keyword **const** means the loop's copy cannot be modified.
2. **std::byte** represents raw binary data; **std::to_integer** converts it to an eight-bit number for arithmetic. The type **std::uint8_t** is an unsigned eight-bit integer.
3. The operator **^=** XORs that byte into the running state. The underscore on remainder_ is a common naming convention for a class member.
4. The inner loop executes eight times; **++bit** increments the bit counter.
5. The operator **& 1u** checks the lowest bit: AND with binary ...0001 leaves only that bit. The **u** suffix marks an unsigned literal.
6. The operator **>>= 1u** changes the state by shifting it right one bit.
7. The conditional **if** applies the specified polynomial only when the old lowest bit was 1.

The class stores its current state between updates. **value()** applies the final XOR without changing that state. **reset()** starts another checksum from the original all-one value.

## 6. Understand the C++ data types

- **std::uint32_t** is a fixed-width unsigned 32-bit integer. We need exactly 32 bits for this checksum.
- **std::array<std::byte, 24>** owns a fixed-size array of 24 bytes. Its maximum is 20 payload bytes plus four checksum bytes.
- **std::span<const std::byte>** is a *view* of existing contiguous memory. It stores access to bytes and their count, but does not own or copy them. Do not retain a span after the source array has died.
- **std::optional<ProtectedPacket>** either contains a packet or contains no result. append_crc uses it to reject an empty or oversized payload. Always check the result before dereferencing.
- **[[nodiscard]]** asks the compiler to warn if a function's return value is ignored.
- **noexcept** states that the function is not expected to throw an exception.
- **std::size_t** is the unsigned type used to represent array sizes and indices.

All these types are part of standard C++; no outside library is needed.

## 7. Big-endian order: a complete worked frame

A 32-bit checksum is stored in four bytes. **Big-endian**, also called **network byte order**, puts the highest-order byte first.

For the ASCII message 123456789 there are nine payload bytes. The checksum is 0xCBF43926. The transmitted trailer is therefore:

    Payload:   31 32 33 34 35 36 37 38 39
    CRC:       CB F4 39 26
    All bytes: 31 32 33 34 35 36 37 38 39 CB F4 39 26

Each pair of hexadecimal digits is one byte. The total is **9 + 4 = 13 bytes**. This is the exact standard vector tested by tests.cpp.

The sender uses shifts of 24, 16, 8, and 0 bits, plus AND with 0xFF, to extract the four checksum bytes. The receiver shifts the accumulated result left eight bits and combines the next byte with bitwise OR. Because both sides agree on the byte order, their numbers match independently of CPU architecture.

## 8. What is streaming?

**Streaming** means processing successive chunks without restarting the calculation. Suppose network data arrives in two pieces: ASCII 123 followed by ASCII 456789.

    integrity::Crc32 running;
    running.update(first_three_bytes);
    running.update(remaining_six_bytes);
    // running.value() is still 0xCBF43926.

An update starts where the previous update ended. Calling value() does not finalize or invalidate the calculator. A subsequent update continues correctly; calling reset() instead starts a completely new message.

The suite tests **every possible two-way split** of the nine-byte reference input, as well as updating one byte at a time.

## 9. Reuse the binary protocol from the last lesson

In Lesson 04, an Add event with order ID 7, price 10123 ticks, and quantity 250 occupies **20 bytes**. A Cancel event occupies **12 bytes**.

The sender first calls the previous wire::encode. It then calls append_crc, which adds exactly four bytes: Add becomes 24 bytes; Cancel becomes 16. The receiver first verifies CRC over all the bytes. Only after a successful check should it pass the **original data bytes, excluding the checksum trailer**, to the previous wire::decode.

Why run both functions? The checksum can be correct even if the packet contains an unsupported version or invalid quantity. CRC detects many accidental changes; the decoder checks the separate rules of the message format.

## 10. Boundaries and what we have not built

The demo accepts 1 to 20 payload bytes and rejects larger payloads. This cap is intentional so the entire protected packet fits inside a 24-byte fixed array. A **bound check** ensures the program does not read or write outside the array.

CRC-32 cannot authenticate the sender. An attacker can change both data and checksum. An accidental **collision** means two different byte strings have the same checksum, which is possible because only 2^32 distinct CRC results exist. CRC also cannot restore damaged data automatically.

This is **not** a TCP stream parser. TCP can divide or merge packet chunks, so a real receiver needs framing and buffering before asking for checksum verification. The implementation is bitwise and optimized for clarity, not measured throughput; no performance claim is made.

## 11. Exercises with answers

1. What size is a 12-byte Cancel packet with a four-byte CRC? **16 bytes.**
2. What size is a nine-byte message with the trailer? **13 bytes.**
3. Does processing 123 and then 456789 produce a different result from processing 123456789? **No.**
4. Does a matching CRC prove that the packet was sent by a trusted person? **No; CRC is not authentication.**
5. Can a matching CRC mean a valid message? **Not by itself; check the protocol fields separately.**
6. What happens if you flip one bit in the sample packet but keep the old trailer? **verify_crc returns false for that example.**

For extra practice, edit main.cpp to demonstrate a Cancel event, calculate its transmitted size, and rerun CTest. Compare a single complete CRC calculation against updates using three arbitrary chunks.

## Further reading

- [CRC-32/ISO-HDLC standard parameters](https://reveng.sourceforge.io/crc-catalogue/all.htm#crc.cat.crc-32-iso-hdlc)
- [C++ std::span](https://en.cppreference.com/w/cpp/container/span)
- [C++ std::byte](https://en.cppreference.com/w/cpp/types/byte)

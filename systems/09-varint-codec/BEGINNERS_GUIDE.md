# Beginner's Guide — Why does 300 need only two bytes?

## 1. The real scenario

An event-processing program receives millions of small numerical identifiers. If every identifier is written as a fixed 64-bit integer, **each one occupies eight bytes** even when its value is 1, 10 or 127.

Sometimes the sender and receiver agree on a variable-length format to reduce message size. Smaller values are encoded in fewer bytes, and the decoder knows when to stop.

## 2. A byte has eight bits

Each bit is 0 or 1. An unsigned byte can represent 0–255, using eight bits. In a base-128 varint, the seven lowest bits hold part of the number. The leftmost/highest bit is a **continuation flag**:

- 0 means this is the final byte.
- 1 means another byte follows.

A value smaller than 128 fits in one byte. For example, 127 is hexadecimal **7F**, with no continuation flag.

## 3. Encode the number 300

Divide by 128 to split the number into seven-bit chunks:

~~~text
300 = 2 * 128 + 44
low chunk  = 44 = 0x2C
high chunk =  2 = 0x02
~~~

The low chunk comes first. Because a second byte follows, set its continuation bit:

~~~text
0x2C | 0x80 = 0xAC
second byte = 0x02
result = AC 02
~~~

To decode, remove the continuation flags and combine the seven-bit chunks:

~~~text
(0xAC & 0x7F) + ((0x02 & 0x7F) << 7)
= 44 + (2 * 128)
= 300
~~~

Shifting left by 7 is the same as multiplying the second chunk by 128.

## 4. Fragmented data is normal

TCP is a byte stream. A complete two-byte varint might arrive split across two network reads:

~~~text
network read 1: AC
network read 2: 02
~~~

After the first byte, the decoder says **need_more**. When the second arrives, it says **complete** and provides 300. It never pretends to have a complete value earlier.

Two values may also arrive together, such as **AC 02 01**. The decoder stops after the first two bytes and reports that two bytes were consumed. The caller can decode the remaining **01** as another value.

## 5. Security and correctness edge cases

A malformed input must not overflow the 64-bit accumulator, read beyond a buffer, or cause unlimited waiting:

- An unsigned 64-bit value requires **no more than ten bytes**.
- The tenth byte may contain only one data bit.
- A sequence like **80 00** is rejected because zero should use only **00**. Accepting multiple encodings of the same integer can cause surprising protocol inconsistencies.
- A caller sending an undersized output span receives an explicit error; no partial value is written.
- You must reset an incremental decoder before starting another integer.

## 6. The C++20 building blocks

- std::uint64_t: always an unsigned 64-bit value.
- std::uint8_t: an eight-bit byte.
- std::span: a non-owning view of memory supplied by the caller.
- std::array: fixed-sized storage used in our tests and demonstration.
- noexcept on functions that cannot throw, and explicit exceptions for invalid encode/output calls.

These choices allow the codec to be integrated into a real event-processing pipeline without forcing a memory allocation for each value.

## 7. Exercises

1. How many bytes are needed for 0? For 127? For 128?
2. Explain why 300 becomes the two hex bytes **AC 02**.
3. Edit the demonstration to encode 16384 and compare its byte count with 16383.
4. What should a decoder do after reading **AC** but before reading **02**?
5. Why should a receiver reject a tenth varint byte of **02**?
6. Add a test demonstrating that encoding into an empty span leaves the buffer untouched.

References: [cppreference — std::span](https://en.cppreference.com/w/cpp/container/span) and [Protocol Buffers — Base 128 Varints](https://protobuf.dev/programming-guides/encoding/#base-128-varints). This lesson uses the same unsigned base-128 idea but does not implement Protocol Buffers as a whole.

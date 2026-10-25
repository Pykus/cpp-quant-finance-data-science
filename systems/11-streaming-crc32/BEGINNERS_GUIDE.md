# Beginner's guide — how a CRC catches accidental changes

When a byte is changed in transit, a receiver may want to detect that
the content is not identical to what the sender calculated.

A CRC maintains a small numerical register that changes as each byte
is processed. Unlike just summing bytes, the bit-shift and polynomial
steps spread information about byte order and positions through the
checksum.

## Understand the types

- `std::byte` represents binary data without suggesting a number
  should be used for arithmetic.
- `std::span<const std::byte>` borrows consecutive bytes from a
  buffer and does not own or copy them.
- `std::uint32_t` is exactly 32 bits; unsigned shifts and wraparound
  have defined behavior.
- `std::string_view` provides a way to process raw text bytes
  without copying or allocating.
- `noexcept` is appropriate because updating uses only fixed-width
  arithmetic and does not allocate.

## Why does splitting input work?

The object stores the *unfinished* internal register between updates.
Calling `update_text("1234")` followed by `update_text("56789")`
is exactly the same sequence of byte operations as processing
`"123456789"` in one call.

The final XOR is applied by `value()`, not to the saved register.
Therefore, inspecting a result does not disturb future updates.

## Practical limitations

Different CRC variants use different polynomials, initialization,
reflection and final XOR rules. A checksum is not secret and does
not prove who created the message. Do not confuse accidental
transmission errors with active attacks.

## Your exercise

Add a table-based CRC and verify that it produces the same reference
vectors. Measure throughput on realistic buffers before claiming
one implementation is faster.

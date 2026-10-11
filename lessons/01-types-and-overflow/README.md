# Lesson 01 — Fixed-width integers and safe conversions

**Goal:** Choose appropriate integer types and avoid accidental data loss when processing sensor readings or hardware register values.

## Why it matters

Embedded systems often exchange values with hardware interfaces that specify exact bit widths. When available, `std::uint8_t` represents an unsigned 8-bit integer (0–255), while `std::uint32_t` represents an unsigned 32-bit integer (0–4,294,967,295). These types are defined in `<cstdint>`.

Converting an unsigned value to a narrower unsigned type discards high-order bits: the result is reduced modulo 2^N, where N is the destination width. Validate the range **before** narrowing if truncation is not intended.

## Build and run

From this directory:

```sh
g++ -std=c++17 -Wall -Wextra -Wconversion -pedantic-errors main.cpp -o lesson01
./lesson01
```

Expected output:

```text
sensor_reading=300
fits_uint8=false
safe_reading=200
safe_as_uint8=200
```

## Key ideas

- `constexpr` makes a function eligible for compile-time evaluation.
- `static_assert` verifies conditions at compile time.
- `fits_in_byte()` checks whether a value can be represented without loss in `std::uint8_t`.
- `static_cast<unsigned>` ensures an 8-bit integer is printed numerically rather than as a character.

## Exercise

Add `constexpr std::uint32_t adc = 1023;` and implement `fits_in_10_bits()` to validate the range 0–1023. Add compile-time assertions confirming that 1023 fits and 1024 does not.

## Common pitfalls

- Do not assume `char` is 8 bits on every platform.
- `volatile` does not provide inter-thread synchronization or replace atomics and memory barriers.
- `std::uint8_t` is commonly an alias of `unsigned char`; streaming it directly may print a character.
- Compile-time checks on example constants do not automatically validate arbitrary runtime input.

## References

- [cppreference — fixed-width integer types](https://en.cppreference.com/w/cpp/types/integer)
- [cppreference — integral conversions](https://en.cppreference.com/w/cpp/language/implicit_conversion)

**Next lesson:** Pointers, references, and memory addresses as foundations for memory-mapped I/O.

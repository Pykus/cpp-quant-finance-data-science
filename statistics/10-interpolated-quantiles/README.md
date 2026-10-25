# Lesson 10 — Empirical quantiles with linear interpolation

Find percentiles in a sample without requiring callers to pre-sort or
modify their data. This C++20 implementation uses the common **type-7**
empirical quantile convention.

## Problem and examples

Input is a nonempty sequence of finite numbers and a probability from
0 to 1 (inclusive). Sort the observations, locate the fractional
position `(n - 1) * probability`, and interpolate between the two
surrounding values.

For samples [4, 1, 3, 2] (sorted: [1, 2, 3, 4]):
- p = 0.00 → 1 (minimum)
- p = 0.25 → 1.75 (first quartile)
- p = 0.50 → 2.50 (median)
- p = 0.75 → 3.25 (third quartile)
- p = 1.00 → 4 (maximum)

The interquartile range is **Q3 minus Q1**, here 1.5.

## Implementation and limitations

The function accepts the vector **by value**, intentionally copying it
before sorting. This avoids unexpected modification of the caller's
original order. It rejects empty samples, nonfinite numbers and invalid
probabilities, and checks for interpolation overflow.

Cost: O(n log n) time to sort and O(n) extra space for a copy.
For very large data, a selection algorithm could be faster if only
one percentile is needed. Quantile conventions differ among statistical
packages; always record which definition you used.

## Build and tests

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Without CMake:
`g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o quantile_tests && ./quantile_tests`

Tests check hand-calculated quartiles, negative and duplicate values,
invalid input and 5,000 deterministic interpolation probabilities.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md). **Exercise:** implement a
nearest-rank quantile and explain why its answers differ for small data.

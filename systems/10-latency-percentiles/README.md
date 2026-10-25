# Lesson 10 — Measure latency tails, not just averages

Low-latency systems must understand their **slowest typical requests**
as well as their central tendency. This C++20 exercise records synthetic
nanosecond durations and reports exact p50, p99 and p99.9 percentiles.

## The contract

Call `LatencyRecorder::add(std::chrono::nanoseconds)` with a
nonnegative duration. `summarize()` returns count, minimum, nearest-rank
p50/p99/p99.9 and maximum, all in integer nanoseconds.

Nearest-rank percentile at probability `p` uses the 1-based rank
`ceil(n*p)`. After sorting, index `rank-1` is the selected sample.
For the integers 1 through 10,000, p50=5,000, p99=9,900, p99.9=9,990.

A single outlier may have little influence on p50 but dominate a
very high percentile in a small sample. Real systems must state
which quantile convention was used.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or use GCC directly:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o latency_tests
./latency_tests
```

Tests cover empty input, negative durations, singleton, outlier,
10,000 descending synthetic observations, percentile ordering
and the exact nearest-rank results.

## Engineering details

- Adding a sample is amortized O(1); memory grows O(n).
- `summarize` copies and sorts the recorded durations, O(n log n).
- It does not mutate the recorder's original sequence.
- `std::chrono::nanoseconds` avoids mixing units accidentally.
- The recorder is **not thread-safe** and must not be concurrently mutated.
- This exercise does not measure operating-system latency or claim
  production trading performance; tests use synthetic values.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md). To reduce memory
usage later, research bounded approximate histograms and quantify
their percentile error.

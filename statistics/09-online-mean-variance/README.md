# Lesson 09 — Online mean and variance (Welford)

Read millions of measurements without storing them all. This C++20 lesson
implements Welford's numerically stable algorithm for mean and variance,
then demonstrates combining independent batches.

## Why not just sum the squares?

The tempting formula `E[x²] - E[x]²` can subtract two very large,
nearly equal numbers. On data such as a trillion plus small fluctuations,
floating-point rounding may destroy the variance.

Welford tracks a count `n`, mean `mean` and sum of squared deviations
`M2`. After receiving a new observation `x`:

```text
delta = x - mean
n = n + 1
mean = mean + delta / n
M2 = M2 + delta * (x - mean)
```

Population variance is `M2/n`. Sample variance is `M2/(n-1)`
and requires at least two observations.

## Input contract and API

`RunningStats` accepts finite floating-point samples via `add`,
with methods `count`, `mean`, `population_variance`,
`sample_variance`, and `merge`.
Nonfinite inputs and numerical overflow raise exceptions before changing state.
A mean for zero samples and sample variance for fewer than two samples
are undefined and throw `std::domain_error`.

## Complexity

`add`, `merge` and queries use O(1) time and O(1) additional memory.
Data can be processed as a stream. This is an educational double-precision
implementation, not an automatic solution to every large-magnitude data set.

## Build and tests

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Without CMake:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o online_tests
./online_tests
```

Tests cover a hand-calculated data set, empty/singleton inputs,
nonfinite observations, streaming 10,000 points, batch merges,
large-offset values, and constant streams.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) and try adding streaming
standard deviation, the square root of the variance.

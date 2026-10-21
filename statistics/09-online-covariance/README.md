# Lesson 9: Streaming Covariance and Correlation in C++20

**Practical problem:** A shop records website visits and paid orders every hour. An analyst needs to know whether both metrics move together. Loading millions of records into memory just to compute one correlation wastes resources.

This original C++20 project updates the means, sample/population covariance, sample variances and Pearson correlation **as pairs arrive**. It uses O(1) memory per stream and provides a mathematically sound merge operation for separately processed batches.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) first for the business story, derivation, annotated example and exercises.

## Build and run

Requires a C++20 compiler. The code has no external dependencies or APIs.

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o demo
    ./demo
    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o tests
    ./tests

CMake 3.16+ may also be used:

    cmake -S . -B build
    cmake --build build
    ctest --test-dir build --output-on-failure

Expected demonstration:

    Weeks: 5
    Sample covariance: 1.500000
    Pearson correlation: 0.774597

## API behavior

- add(x, y): O(1) update for the next paired observation.
- merge(other): combine summaries computed from disjoint batches without saving all raw rows.
- sample_covariance(): the centered cross-product divided by n - 1.
- population_covariance(): the cross-product divided by n.
- variance_x() and variance_y(): unbiased sample variances.
- correlation(): Pearson's dimensionless linear correlation in [-1, 1].
- Undefined estimates return std::nullopt (empty data, fewer than two points, or a constant series).
- NaN, infinity and overflowing state updates are rejected without changing previously accumulated values.

The tests cover hand-calculated answers, perfect positive/negative correlation, zero variance, batch merges, invalid observations, numeric offsets and overflow.

## Production boundaries

Pearson correlation is *not* evidence of causation. Paired values must refer to aligned times; missing readings must be handled upstream, and the class itself is not synchronized for concurrent mutation. Finite floating-point precision remains a limitation for extreme scales. Example figures are invented.

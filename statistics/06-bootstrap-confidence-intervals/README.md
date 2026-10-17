# Bootstrap confidence intervals for means and medians

## Why this matters

A call-center manager recorded six waiting times: **3, 5, 4, 7, 6, 9 minutes**. Their average is **5.67 minutes** and median **5.5 minutes**. But six calls are only a sample, not the entire population. How uncertain is either estimate?

This C++17 project implements the **bootstrap percentile interval**. It creates thousands of new samples **with replacement**, computes an average or median for each, then uses the 2.5th and 97.5th percentiles as approximate 95% limits.

**Practical purpose:** provide a reproducible estimate of sampling uncertainty for a small measurement dataset. All values are fictional. The method does **not** predict the range of individual future calls.

## Steps and expected results

1. Start with the six measured times.
2. Draw another six from the original observations, allowing duplicates.
3. Calculate the mean (or median) of the new sample.
4. Repeat the draw **2,000 times**.
5. Sort all resulting statistics and take the lower and upper percentiles.

The demo prints an observed mean of approximately **5.67 minutes**, an observed median of **5.50 minutes**, and their two estimated confidence intervals. The pseudorandom seed makes the calculation repeatable with the same C++ standard library implementation.

## Build without external dependencies

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o bootstrap_demo
./bootstrap_demo
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic tests.cpp -o bootstrap_tests
./bootstrap_tests
```

Or use CMake where installed:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## API

```cpp
const std::vector<double> waits{3, 5, 4, 7, 6, 9};
const auto interval = stats::bootstrap_interval(
    waits, stats::Statistic::mean, 2000, 0.95, 42);
// interval.point_estimate, interval.lower, interval.upper
```

The header validates the data: at least two finite observations, at least 100 resamples, and confidence strictly between 0 and 1. It returns an error for bad input. The median works for even or odd sample sizes, and percentile boundaries use linear interpolation.

**Limitations:** The percentile bootstrap can be inaccurate for small, highly skewed or discrete samples. Independent, representative observations are assumed. It does not handle serial correlation, weighting or BCa bias corrections. The interval is an estimate for a population statistic, not a guarantee of coverage for this one tiny example.

Start with the plain-language [beginner's guide](BEGINNERS_GUIDE.md) for formulas, C++ code, worked resampling examples and exercises.

# Lesson 06: A bootstrap confidence interval for a mean

**Question:** How uncertain is a sample average if we cannot observe the whole population?

Start with the [first-year student's guide](BEGINNERS_GUIDE.md). This lesson extends the [standard error of the mean](../05-standard-error-of-mean/) by simulating a sampling distribution rather than simply using a formula.

## From five observations to an interval

Our fictional readings are **8, 10, 12, 14, 16**. Their mean is **12**.
One bootstrap resample might be `[8, 8, 10, 16, 16]`. We select **five values with replacement**, calculate its mean, and repeat this many times.
Sorting 10,000 simulated means gives the 2.5th and 97.5th percentiles: endpoints of a **95% percentile bootstrap interval**.

The bootstrap standard error is the sample standard deviation of these resampled means. It is not the raw-data standard deviation.

## Build and test

Run from this lesson directory:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror bootstrap_mean.cpp main.cpp -o bootstrap_demo
./bootstrap_demo
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror bootstrap_mean.cpp tests.cpp -o bootstrap_tests
./bootstrap_tests
```

CMake alternative:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The demonstration reports mean, bootstrap SE, confidence limits and count. A fixed seed allows repeated results for the same standard-library implementation; numeric endpoints can differ between implementations.

## Source code map

- `bootstrap_mean.hpp`: typed result and public function.
- `bootstrap_mean.cpp`: input checking, resampling, stable accumulation, quantiles.
- `main.cpp`: synthetic five-reading example.
- `tests.cpp`: determinism, numerical properties, validation, boundaries.
- `BEGINNERS_GUIDE.md`: full worked calculations, terminology, C++, exercises and answers.

## Assumptions

Observations should be independent, identically sampled, and representative of the target population. This simple percentile method can have poor actual coverage with small samples, very skewed or heavy-tailed distributions, and statistics near constraints. Resampling does not fix selection bias. For dependent financial time series, look into **block bootstrapping**. More advanced intervals include BCa or studentized bootstrap.

The phrase "95% confidence" describes intended **long-run coverage of a method**, not a 95% probability attached to a fixed population mean after observing one particular interval. All example data are synthetic.

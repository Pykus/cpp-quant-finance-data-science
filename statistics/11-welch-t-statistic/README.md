# Lesson 11 — Welch's two-sample t-statistic

Compare two independent numerical samples even when their variances and
sample sizes differ. C++20 `std::span` lets the function read arrays,
vectors or other contiguous storage without copying.

## What this computes

For groups A and B with sample means `mean_a`, `mean_b`,
sample variances `v_a` and `v_b`:

```text
standard_error = sqrt(v_a/n_a + v_b/n_b)
t = (mean_a - mean_b) / standard_error
df = (v_a/n_a + v_b/n_b)^2 /
     ((v_a/n_a)^2/(n_a-1) + (v_b/n_b)^2/(n_b-1))
```

The final equation gives approximate **Welch–Satterthwaite degrees of
freedom**, which need not be an integer.

For A=[1,2,3] and B=[2,3,4], the difference is -1,
standard error ≈ 0.81650, t ≈ -1.22474 and df = 4.

## Assumptions

The observations should be independent within and across groups for
the usual inferential interpretation. The algorithm rejects nonfinite
inputs, fewer than two observations per group and a zero combined
standard error.

**It does not calculate a p-value or establish a causal claim.**
For actual hypothesis testing, consider sampling design, confidence
intervals, multiplicity and the appropriate t-distribution.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or: `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o welch_tests && ./welch_tests`.

Welford's stable mean/variance update takes O(n) time and O(1)
extra space. Tests cover hand-calculated samples, group reversal,
unequal sample sizes, invalid inputs and large deterministic samples.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) and try comparing
groups with unequal means **and** unequal variances.

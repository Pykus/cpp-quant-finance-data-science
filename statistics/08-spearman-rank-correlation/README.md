# Lesson 08: Spearman rank correlation with tied observations

**Practical question:** If customer satisfaction scores are higher, do repeat-visit intentions generally rank higher too? The synthetic survey pairs in `main.cpp` demonstrate an association calculation; they do not establish causation.

Spearman's rho is the ordinary Pearson correlation of **ranked** values. The implementation:

- Preserves input pairings and assigns **midranks** to ties (for instance, equal first and second positions receive rank 1.5).
- Uses `long double` accumulators for rank covariance and variance; clamps the final floating-point result to [-1, 1].
- Reports the number of original pairs and the number of observations belonging to tied groups in each variable.
- Rejects unpaired lengths, fewer than two paired observations, nonfinite values, and constant-ranked series instead of returning misleading numbers.
- Does not estimate p-values, confidence intervals, or make causal claims; nonrandom or small samples require additional scrutiny.

The shortcut `1 - 6 * sum(d*d)/(n*(n*n-1))` is **not appropriate with ties**. This implementation always correlates the ranks directly.

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/spearman_demo
```

Or use `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o spearman_tests && ./spearman_tests`.

All input data are synthetic. Each survey record is a matched `(x, y)` pair in the same vector position, and missing values must be handled by an explicit, documented policy *before* calling this function. Do not silently drop observations from just one series.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for worked examples, mathematics from the beginning, the meaning of ties and exceptions, and exercises.

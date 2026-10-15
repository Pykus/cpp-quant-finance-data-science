# Standardizing data with z-scores (C++17)

**Question:** How far is one measurement from the average, measured in units of typical variation?

This lesson extends [descriptive statistics](../01-descriptive-statistics/) and
[covariance and correlation](../02-covariance-correlation/). You will compute a
sample mean, a sample standard deviation, and a **z-score** for each number.
For a first-principles explanation, start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md).

## Vocabulary

- **Observation:** one recorded number.
- **Sample:** a collection of observations, often part of a larger population.
- **Mean:** the sum of values divided by their count.
- **Deviation:** one value minus the mean.
- **Sample variance:** squared deviations added together, divided by `n - 1`.
- **Sample standard deviation:** the square root of sample variance; it uses the original units.
- **Z-score:** `z = (value - mean) / sample_standard_deviation`. It has no units.
- **Standardization:** replacing each value with its z-score to compare relative positions.

A positive z-score means above the sample mean; a negative one means below it.
A z-score of 1.0 means one sample standard deviation above the mean.

## Worked example (synthetic measurements)

| Observation | Deviation from 14 | Deviation squared | Z-score |
| ---: | ---: | ---: | ---: |
| 10 | -4 | 16 | -1.264911 |
| 12 | -2 | 4 | -0.632456 |
| 14 | 0 | 0 | 0 |
| 16 | 2 | 4 | 0.632456 |
| 18 | 4 | 16 | 1.264911 |

Mean = `(10 + 12 + 14 + 16 + 18) / 5 = 14`.
Sum of squared deviations = `16 + 4 + 0 + 4 + 16 = 40`.
Sample variance = `40 / (5 - 1) = 10`.
Sample standard deviation = `sqrt(10) = 3.162278`.
Therefore, the last value's z-score is `(18 - 14) / 3.162278 = 1.264911`.

## Build and run (no external libraries)

From this directory, with a C++17-compatible compiler:

~~~bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror z_scores.cpp main.cpp -o zscore_demo
./zscore_demo
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror z_scores.cpp tests.cpp -o zscore_tests
./zscore_tests
~~~

Expected demonstration (rounded to six decimals):

~~~text
Sample mean: 14.000000
Sample standard deviation: 3.162278
Value 10.000000 -> z-score -1.264911
Value 12.000000 -> z-score -0.632456
Value 14.000000 -> z-score 0.000000
Value 16.000000 -> z-score 0.632456
Value 18.000000 -> z-score 1.264911
~~~

## Code map

- `z_scores.hpp`: two function declarations and the result structure.
- `z_scores.cpp`: validate finite inputs, calculate mean, sample spread, then z-scores.
- `main.cpp`: synthetic five-number example with readable printed output.
- `tests.cpp`: arithmetic checks, transformation properties, and invalid-input checks.

The program rejects fewer than two observations, any NaN/infinite values,
and samples with zero standard deviation. Floating-point rounding can occur.
Standardization does **not** make a data set normally distributed and does not
make an outlier harmless. Fit the mean and standard deviation on training data
only if using this technique in a predictive model; otherwise you risk data leakage.

## Exercises

1. Calculate z-scores manually for `2, 4, 6` and compare with the program.
2. Add 100 to every sample value. What happens to the z-scores? Why?
3. Double every sample value. What happens? Why?
4. Decide what your application should do with a sample `7, 7, 7`.

Answers and guided reasoning: [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md).

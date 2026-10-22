# Beginner's guide: What Spearman correlation actually measures

This lesson assumes only arithmetic, averages, and the idea of putting numbers in order. It uses fictional measurements throughout. You do **not** need to know C++ or a statistics library to understand the mathematical examples.

## 1. The question behind the formula

Suppose we ask four customers two questions: "How satisfied are you?" and "How likely are you to return?" Each customer supplies two answers. We want to know whether the **ordering of customers** is similar for both answers.

- A positive association means relatively high satisfaction tends to accompany relatively high intention.
- A negative association means relatively high satisfaction tends to accompany relatively low intention.
- No ranked association means there is no consistent *monotonic* ordering in these observations.

An association alone does **not** prove the first variable causes the second. Another variable might influence both, people might have been selected unrepresentatively, or the apparent effect might be chance variation.

## 2. Paired data: do not rearrange one column independently

Each row belongs to one person:

| Customer | Satisfaction X | Return intention Y |
| --- | ---: | ---: |
| A | 1 | 1 |
| B | 2 | 3 |
| C | 3 | 2 |
| D | 4 | 4 |

There are **n = 4 paired observations**. If we sort X independently from Y, the customers will no longer match. We must keep the identity of each pair even when ranking values.

## 3. Ranking instead of using the raw numbers

Give the smallest observation rank 1, the second-smallest rank 2, and so on. Both columns above already contain the numbers 1 through 4, so the ranks are the same as the values:

| Customer | Rank(X) | Rank(Y) |
| --- | ---: | ---: |
| A | 1 | 1 |
| B | 2 | 3 |
| C | 3 | 2 |
| D | 4 | 4 |

The ranks say *relative position*, not how many points one measurement is above another. X = [1, 2, 3, 4] has exactly the same ranks as X = [1, 10, 100, 1000]. This matters when a scale is ordinal or a relationship is monotonic but nonlinear.

## 4. Pearson correlation, applied to ranks

Pearson's correlation for numbers `a[i]` and `b[i]` is:

```
                  sum((a[i] - mean(a)) * (b[i] - mean(b)))
r = -----------------------------------------------------------------
    sqrt(sum((a[i] - mean(a))^2) * sum((b[i] - mean(b))^2))
```

Spearman's rank correlation, called **rho** (the Greek letter ρ), is the same formula applied to rank(X) and rank(Y). The division by `n` or `n-1` that appears in some definitions of covariance cancels between numerator and denominator, so the expression above works directly.

For four ranks, the mean is `(1+2+3+4)/4 = 2.5`. Subtract 2.5 from each rank to get the deviations:

| Customer | Rank(X) - 2.5 | Rank(Y) - 2.5 | Product |
| --- | ---: | ---: | ---: |
| A | -1.5 | -1.5 | 2.25 |
| B | -0.5 | +0.5 | -0.25 |
| C | +0.5 | -0.5 | -0.25 |
| D | +1.5 | +1.5 | 2.25 |

The products add to `4`. The squared deviations in either column add to `5`. Therefore:

```
rho = 4 / sqrt(5 * 5) = 0.8
```

Interpretation: the two rankings have a fairly strong positive association in these **four** fictional observations. A four-person example is not enough to infer a population relationship with much confidence.

## 5. What is a tie?

A **tie** occurs when two or more values are exactly equal. Example:

```
Values:  10  10  20  30
Ranks:  1.5 1.5  3   4
```

The first two observations occupy positions 1 and 2, so each gets the **midrank** `(1 + 2)/2 = 1.5`.

Another example:

```
Values:  9  7  9  7  7
Ranks: 4.5 2 4.5 2  2
```

Three 7s occupy rank positions 1, 2, and 3, giving each `(1+2+3)/3 = 2`. Two 9s occupy positions 4 and 5, giving each 4.5.

A ranking with midranks still has the same mean rank `(n+1)/2`. That is why the implementation can calculate the mean directly rather than summing ranks again.

### Worked example with ties

Take X = `[1, 1, 2, 3]` and Y = `[1, 2, 2, 3]`.

```
Rank(X) = [1.5, 1.5, 3,   4  ]
Rank(Y) = [1,   2.5, 2.5, 4  ]
Mean    = 2.5 in both columns
```

Deviations for X: `[-1, -1, 0.5, 1.5]`. Deviations for Y: `[-1.5, 0, 0, 1.5]`.

The cross-products sum to `3.75`. Each sum of squared deviations is `4.5`. Thus rho = `3.75 / 4.5 = 0.833333...`.

A frequently published shortcut formula is:

```
rho = 1 - 6 * sum((rankX-rankY)^2) / (n * (n^2-1))
```

**That shortcut assumes there are no ties.** When ties appear, using it unchanged is incorrect. Our implementation uses the general Pearson-on-midranks method for every input, with and without ties.

## 6. Interpreting numbers between -1 and +1

- `+1`: rankings match perfectly (possibly with matching tie groups).
- `-1`: one ranking is exactly opposite the other.
- `0`: no *linear correlation of ranks* in this sample. This does not mean two variables are independent.
- `+0.5`: a moderately positive ranked association, but the strength label is context-dependent.
- `-0.5`: a moderately negative ranked association.

With only two distinct paired observations and no ties, rho must be `+1` or `-1`; this is a mathematical limitation, not strong scientific evidence. Even rho near 1 does not tell you how much Y changes when X rises one unit.

## 7. Spearman versus ordinary Pearson correlation

**Ordinary Pearson** works with the original numbers and measures how well a straight-line relationship fits. **Spearman** works with ranks, so it detects any consistently increasing or decreasing *monotonic* pattern.

Example: X = `[1, 2, 3, 4]`, Y = `[1, 4, 9, 16]` is a curved relationship, but both rankings are perfectly increasing. Spearman gives `+1`, while ordinary Pearson is less than `+1`.

Ranks also reduce the influence of extreme magnitude. Replacing Y = 16 with Y = 160000 leaves the order and Spearman rho unchanged, although the ordinary Pearson coefficient changes.

But ranks do not cure unrepresentative sampling, missing data, hidden confounding, incorrectly paired rows, or measurement bias.

## 8. Input validation and undefined cases

The C++ function takes two vectors of `double`: `spearman(x, y)`.

1. They must have exactly the same length: each index represents one matched pair.
2. There must be **at least two** paired observations.
3. Every value must be a finite real number. NaN and infinity are rejected explicitly.
4. If all X values are the same, X has zero rank variance. Correlation is undefined.
5. The same applies when all Y values are identical.

Why is a constant series invalid? Correlation divides by the product of standard-deviation-like terms. A constant series has zero variance, which would make the denominator zero. Reporting "zero association" would be misleading; an exception is safer.

The program intentionally does **not** auto-delete records with missing answers. A careful analyst must choose a missing-data policy, document it, and maintain X/Y pairing. If you remove a respondent, remove the **entire matched row** rather than one cell.

## 9. How the code works, step by step

In `midranks`:

1. Create indices `0, 1, 2, ...` rather than sorting the original vector.
2. Validate every number with `std::isfinite`.
3. Sort the indices by the corresponding values.
4. Group exactly equal values (`==`) in sorted order.
5. Give each group the average occupied rank; write these ranks back at the original indices.
6. Count observations that belong to tied groups, not just the number of groups.

In `spearman`:

1. Validate matched lengths and minimum sample size.
2. Rank the two series independently, retaining the original row indices.
3. Find the rank mean `(n+1)/2`.
4. Calculate `cross`, `xx`, and `yy` using `long double` accumulators.
5. Reject a zero variance (`xx == 0` or `yy == 0`).
6. Return `cross / sqrt(xx*yy)` together with metadata.

Using `long double` reduces intermediate rounding on supported platforms; it does not make calculations infallible. The result is clamped into the mathematically valid [-1, +1] interval in case floating-point rounding produces a value just outside it.

**Complexity:** Ranking two vectors with sorting takes O(n log n) time, and the extra index/rank vectors need O(n) memory. Calculating the correlation after ranking takes O(n) time. The method is deterministic, with no random sampling.

## 10. How to compile and run tests

From this directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/spearman_demo
```

The test executable has a small dependency-free assertion harness. Each assertion is counted, and any failed assertion exits the executable with a nonzero status. `ctest` treats that as a failed test. You can also compile tests without CMake:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o spearman_tests
./spearman_tests
```

Do not copy claims about passing tests into a report until you have actually executed them on the version you intend to publish.

## 11. Practice problems

**Exercise A.** X = `[1, 2, 3]`; Y = `[3, 2, 1]`. What is rho?

**Exercise B.** Values are `[5, 5, 5, 7]`. Which ranks do they receive?

**Exercise C.** If 100 becomes 1,000,000 but remains the largest observation, must Spearman change?

**Exercise D.** Why should `spearman({1,1,1}, {1,2,3})` throw rather than return 0?

**Exercise E.** Imagine Y doubles every time X increases. Is the relationship necessarily a straight line? Will Spearman still be +1 if both are strictly increasing?

**Exercise F.** If 30 people all answer "5" to satisfaction and give varied intentions, can you calculate a meaningful Spearman rho?

### Suggested answers

A: `-1` because the order is exactly reversed.

B: `[2, 2, 2, 4]` because the three equal entries occupy positions 1-3.

C: No. The ranks are unchanged as long as relative order and ties are unchanged.

D: The rank variance of X is zero, so the correlation denominator is zero. Zero correlation would describe a different, defined situation.

E: Doubling is generally exponential, not linear. Yes, Spearman rho is +1 if the ranking is strictly increasing for every pair.

F: No. Satisfaction has no variation and its ranked variance is zero.

## 12. Limits for real research

- A high rho is not a prediction model and is not proof of causation.
- A sample statistic is not automatically a population statistic.
- A small sample can give deceptively extreme correlations; uncertainty assessment matters.
- Heavy ties (e.g. five-point Likert answers) reduce available distinct ranks. Report that ties are present.
- A pattern that rises and then falls can yield rho near zero despite a strong nonlinear relationship.
- Screening numerous pairs and showing only the largest correlations can create selection bias.
- Significance tests for rho require carefully chosen assumptions and often special treatment of ties; this lesson deliberately does not invent a p-value.

**Next step:** draw a scatter plot of the paired observations, discuss why a monotonic relationship is or is not plausible, and only then choose further inference methods.

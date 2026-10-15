# Beginner's guide: what a z-score really means

This guide assumes you know how to add, subtract, divide and take square roots.
You do not need a statistics course, finance knowledge, or previous C++ experience.

## 1. Start with five measurements

Imagine recording five **synthetic** measurements: **10, 12, 14, 16, 18**.
An **observation** is one measurement. A **sample** is our list of observations.
The **sample size**, usually called `n`, is the number of observations: here, `n = 5`.
These measurements are made up solely to help you learn.

A **mean** is an arithmetic average: add all numbers and divide by the count.

~~~text
sum = 10 + 12 + 14 + 16 + 18 = 70
mean = 70 / 5 = 14
~~~

The **center** of this particular sample is therefore 14.

## 2. Why does the mean alone hide information?

These two samples share mean 14:

- A: `13, 14, 15` (measurements huddle near 14)
- B: `4, 14, 24` (measurements are far apart)

To quantify the typical **spread** around the mean, first compute each
**deviation**: `observation - mean`. A deviation can be negative.

For our five measurements:

~~~text
10 - 14 = -4
12 - 14 = -2
14 - 14 =  0
16 - 14 =  2
18 - 14 =  4
~~~

If we add signed deviations, positive and negative values cancel to zero.
Instead, **square** them (multiply each by itself) to get nonnegative values:

~~~text
(-4)^2 = 16
(-2)^2 = 4
   0^2 = 0
   2^2 = 4
   4^2 = 16
total = 40
~~~

## 3. Population variance versus sample variance

A **population** means all the observations in the group we care about.
A **sample** is only part of that larger group, used to estimate its properties.

If the five values are the **entire population**, the population variance is
`40 / 5 = 8`. But we deliberately treat our observations as a **sample**.

For the familiar **sample variance**, divide by `n - 1` instead of `n`:

~~~text
sample variance = 40 / (5 - 1) = 40 / 4 = 10
~~~

The `n - 1` denominator is known as **Bessel's correction**. It compensates
for estimating the population mean using the same sample (under standard
independent-sampling assumptions). This lesson uses sample statistics
consistently, not population statistics.

**Standard deviation** means the square root of variance:

~~~text
sample standard deviation = sqrt(10) = 3.162277660...
~~~

The square root returns the result to the original measurement units.
The three dots mean additional decimal digits exist.

## 4. Standardize a measurement

We want to compare how unusual a measurement is *relative to this sample*.
Calculate the **z-score**:

~~~text
z-score = (observation - sample mean) / sample standard deviation
~~~

Take observation 18:

~~~text
z(18) = (18 - 14) / sqrt(10)
      = 4 / 3.162277660...
      = 1.264911064...
~~~

This says 18 is about **1.265 sample standard deviations above the mean**.
For observation 10, the z-score is approximately -1.265.
Observation 14 is exactly at the sample mean, so its z-score is zero.

The z-score is **dimensionless**: both the numerator and denominator use the
same original units, so those units cancel.

## 5. What do the signs and sizes mean?

- `z = 0`: exactly at the sample mean.
- `z > 0`: above the sample mean.
- `z < 0`: below the sample mean.
- `z = 2`: two sample standard deviations above the mean.

Standardizing the five values gives approximately
`-1.264911, -0.632456, 0, 0.632456, 1.264911`.

Their sample mean is approximately zero and their **sample**
standard deviation is approximately one. “Approximately” accounts for
floating-point rounding in computers.

**Important:** z-scores do not tell you the probability of an observation
unless you also have an appropriate statistical model or distribution.
A **normal distribution** is a specific bell-shaped probability model;
standardization alone does not turn arbitrary data into a normal distribution.
A very large absolute z-score may deserve investigation, not automatic deletion.

## 6. The same idea in beginner-level C++

Open [z_scores.hpp](z_scores.hpp) and [z_scores.cpp](z_scores.cpp).
Here is a small guided map of the code.

- `#include <vector>` lets us store a variable-length list of numbers.
- `std::vector<double>` is such a list; `double` stores fractional numbers.
- `const std::vector<double>& observations` means “read a supplied list without
  copying it or changing it.” The `&` indicates a reference.
- `observations.size()` is the number of values, our `n`.
- `if (observations.size() < 2)` protects the sample denominator `n - 1`.
- `for (const double value : observations)` visits every number once.
- `std::isfinite(value)` rejects NaN (not-a-number) and positive/negative infinity.
- `long double sum` uses a wider floating-point type for intermediate arithmetic
  on platforms where `long double` has greater precision.
- `static_cast<long double>(value)` explicitly converts a `double` to `long double`.
- `sum / count` calculates the sample mean.
- `difference * difference` is the squared deviation.
- `std::sqrt(variance)` calculates the square root.
- `throw std::invalid_argument(...)` reports invalid input by raising an
  **exception**, an error that a caller can catch and handle.
- `std::domain_error` signals that no meaningful z-score exists for a constant
  sample (zero spread), or a result cannot be represented.
- `standardized.reserve(observations.size())` reserves space in advance to avoid
  unnecessary internal growth of the output list.
- `push_back(...)` appends each calculated z-score to the output list.
- `return standardized;` returns the completed list to the caller.

The function `describe_sample` computes the mean and sample standard
deviation. The function `z_scores` reuses those results to standardize each
observation. This separation keeps one job per function.

In [main.cpp](main.cpp), the program defines a short synthetic list,
calls both functions, and prints each result. The line using
`std::setprecision(6)` shows six decimal places. It does not change the
underlying floating-point calculations.

## 7. Work through the source yourself

Trace the program with the five values, writing down the variables:

| After this step | Value |
| --- | ---: |
| Number of observations | 5 |
| `sum` | 70 |
| `mean` | 14 |
| `sum_of_squared_deviations` | 40 |
| `variance` | 10 |
| `standard_deviation` | 3.162277660... |
| First z-score | -1.264911064... |
| Last z-score | 1.264911064... |

These variable names correspond directly to the program.
Next, replace the measurements with `2, 4, 6` and predict the output
before running the program.

## 8. Boundary cases and good programming habits

**One observation:** You cannot estimate a nonzero sample spread with the
`n - 1` formula. The code rejects an input size smaller than two.

**All observations equal:** With `7, 7, 7`, the mean is 7 and every
deviation is zero. Standard deviation is zero. Dividing by zero would not
yield a meaningful z-score, so the program raises an exception instead.

**NaN or infinity:** Such special floating-point inputs cannot be treated
as ordinary measurements; the code rejects them.

**Precision:** Computers store most fractional numbers approximately.
Always compare computed floating-point values with a reasonable tolerance
in tests instead of assuming bit-for-bit equality.

The test program checks normal results, the behavior when numbers are
shifted or scaled, and invalid inputs. Reading and changing tests is a
great way to learn.

## 9. Exercises with answers

**Exercise A:** Find z-scores for `2, 4, 6`.

Mean `= 4`, squared deviations `= 4 + 0 + 4 = 8`, sample variance
`= 8 / (3 - 1) = 4`, sample standard deviation `= 2`.
Therefore z-scores are **-1, 0, 1**.

**Exercise B:** Add 100 to each observation.

Both values and mean increase by 100, leaving every deviation unchanged.
The sample standard deviation and z-scores therefore stay the same.

**Exercise C:** Multiply each observation by 2.

Each deviation and the sample standard deviation double, so the ratio
(z-score) stays the same. Multiplying by a *negative* number reverses
the z-score signs.

**Exercise D:** What should happen for `7, 7, 7`?

There is no variability: sample standard deviation is zero. In this lesson,
throwing a clear exception is safer than quietly returning 0 or infinity.

## 10. What to learn next

The earlier [descriptive statistics](../01-descriptive-statistics/) lesson
introduces averages and variance. The
[covariance and correlation](../02-covariance-correlation/) lesson
extends centering to two variables. The
[linear regression](../03-linear-regression/) lesson studies a fitted line.

A useful next project is to standardize a **training** data set, save its
mean and standard deviation, and reuse *those same* values for new
observations. That avoids **data leakage**: the mistake of using
information from unseen future/test data while fitting a model.

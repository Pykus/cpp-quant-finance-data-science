# Beginner's guide: how uncertain is an average?

This lesson assumes only basic arithmetic and the ability to follow simple C++.
You do not need a previous statistics course. We will start with actual
numbers, explain every new technical word, and then connect them to the program.

## 1. Why an average may change

Suppose we take five **fictional** measurements: `8, 10, 12, 14, 16`.
A **measurement**, also called an **observation**, is one recorded value.
A **sample** is the short list we collected. Our **sample size** is five,
written `n = 5`. The **population** means the larger collection from which
we wish to learn. We rarely have every population observation.

The **sample mean** (arithmetic average) is the sum divided by count:

```text
(8 + 10 + 12 + 14 + 16) / 5 = 60 / 5 = 12
```

If we drew a different sample from the same population, its average might
be 11 or 13 rather than exactly 12. This is **sampling uncertainty**:
the estimate varies because we only observed part of the population.

## 2. Individual variability: sample standard deviation

A **deviation** means a value minus our average. Deviations here are:
`-4, -2, 0, 2, 4`. Because positive and negative deviations cancel,
we **square** each one (multiply it by itself) before adding them.

```text
value              8    10    12    14    16
value minus 12    -4    -2     0     2     4
squared amount    16     4     0     4    16
total squared deviation = 40
```

**Sample variance** is the squared-deviation sum divided by `n-1`.
For five observations we divide by four: `40 / 4 = 10`.
Using `n-1` instead of `n` is called **Bessel's correction**; under
ordinary independent sampling it corrects the variance estimate for the
fact that we also estimated the mean from these observations.

**Sample standard deviation**, abbreviated `s`, is the square root
of sample variance: `s = sqrt(10) = 3.162277660...`.
A **square root** is the positive number that, multiplied by itself,
returns the original number. The notation `sqrt(10)` means that number.

Sample SD tells us about the spread of **individual readings**.
It does NOT directly tell us how much the estimated mean would change
between different random samples.

## 3. Average variability: standard error

The **standard error of the mean**, abbreviated `SE`, estimates the typical
variation of the calculated mean between hypothetical random samples,
assuming independent observations from a common population.

```text
SE = sample standard deviation / square root of sample size
SE = s / sqrt(n)
SE = sqrt(10) / sqrt(5)
SE = sqrt(2)
SE = 1.414213562...
```

Our observations have sample SD approximately **3.162**, but the estimated
mean has standard error approximately **1.414**. The distinction matters:
one is about values; the other is about averages.

### Why divide by sqrt(n), not by n?

Averages tend to fluctuate less than individual measurements, because
some high values and low values cancel. For independent observations with
a common finite variance, the variance of the average is the individual
variance divided by `n`. Taking square roots gives the `sqrt(n)` rule.

For the *same underlying variability*, quadrupling the independent sample
size roughly halves its SE. For example, increasing `n` from 5 to 20
changes `s/sqrt(5)` to `s/sqrt(20)`, if `s` stayed the same.
This is an approximate comparison; actual sample SD also changes.

Repeating the **same** sensor value does not create independent information.
Repeated readings that influence one another are called **correlated**
observations and may make `s/sqrt(n)` too optimistic.

## 4. Standard error is not a confidence interval

A **confidence interval** is an interval produced by a method designed to
contain the population parameter a specified proportion of the time across
repeated sampling. Computing one needs additional assumptions, a critical
value, and possibly small-sample adjustments. This lesson computes SE,
not a confidence interval or a probability that the true mean is correct.

SE only represents random sampling variation under its model. A faulty
sensor (**measurement bias**) or nonrepresentative selection (**selection
bias**) can still distort an average. Collecting more biased measurements
does not make the estimated mean trustworthy.

## 5. Connect the math to C++

Open [standard_error.hpp](standard_error.hpp).

- `#pragma once` means include this declaration file only once per source.
- `#include <vector>` gives access to `std::vector`, a resizable list.
- `#include <cstddef>` defines `std::size_t`, an unsigned counting type.
- `namespace statistics` groups our declarations under a clear name.
- `struct MeanUncertainty` creates a new record type with four fields:
  count, mean, sample standard deviation and SE.
- `double` is a common binary floating-point number type: most decimal
  results are approximations rather than exact fractions.
- `[[nodiscard]]` asks compilers to warn if the returned result is ignored.
- `const std::vector<double>& sample` means the function receives the
  original list by reference and promises not to modify it.

Now open [standard_error.cpp](standard_error.cpp).

### Input checks

`if (sample.size() < 2)` rejects a sample too small for the `n-1`
variance denominator. `throw std::invalid_argument(...)` reports a
caller mistake using an **exception** (an error that interrupts normal
execution). `std::isfinite` rejects `NaN` ("not a number") and positive
or negative **infinity**, which are special floating-point values.

### Running the calculation safely

The implementation uses **Welford's update**, a numerical technique that
tracks a running mean and squared-deviation sum without subtracting large,
nearly equal totals. It is an alternative to the two-pass formula we
calculated by hand; it estimates the same mathematical sample variance.

For each new `value`:

```cpp
++count;
const double delta = value - mean;
mean += delta / static_cast<double>(count);
const double updated_delta = value - mean;
squared_deviation_sum += delta * updated_delta;
```

- `++count` increases the number processed by one.
- `delta` measures the new reading's difference from the OLD running mean.
- `static_cast<double>(count)` converts the integer count to floating point
  so the division is carried out in floating-point arithmetic.
- `mean += ...` adjusts the average when adding one new observation.
- `updated_delta` measures the reading from the NEW running mean.
- The final line updates a running sum from which sample variance is
  calculated. It avoids some loss of numerical precision for values
  clustered around a very large starting number.

After the loop, the program divides by `n-1`, takes `std::sqrt` to find SD,
and divides once more by `sqrt(n)` to get SE. It checks for arithmetic
overflow (numbers too large for `double` to represent).

### Output

Open [main.cpp](main.cpp). A `std::vector<double>` stores the five
fictional values; `statistics::summarize_mean(measurements)` performs the
calculation. `std::cout` prints text; `std::fixed` and
`std::setprecision(6)` print six digits after the decimal point.
`'\\n'` prints a newline. The expected output is:

```text
Observations: 5
Sample mean: 12.000000
Sample standard deviation: 3.162278
Standard error of the mean: 1.414214
```

## 6. Run the code and read test results

From this folder, run these commands in a terminal (a text-based command
interface):

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror standard_error.cpp main.cpp -o mean_demo
./mean_demo
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror standard_error.cpp tests.cpp -o mean_tests
./mean_tests
```

`g++` is the GNU C++ **compiler** that translates human-readable source
into an executable. `-std=c++17` selects the language version.
`-Wall -Wextra -Wpedantic` enable useful compiler warnings and
`-Werror` treats warnings as errors. `-o` names the executable file.

A **test** compares a calculated answer with a known expected answer.
Because floating-point arithmetic rounds, tests compare real numbers
within a small **tolerance**, such as `1e-12` (one trillionth).

## 7. Exercises, with hints and answers

**Exercise A:** For independent observations `10,14`, first find the mean.
Answer: `12`. The deviations are `-2,+2` and their squared sum is `8`.
Sample variance `8/(2-1)=8`, SD `sqrt(8)`, SE `sqrt(8)/sqrt(2)=2`.

**Exercise B:** Multiply every reading in `8,10,12,14,16` by two.
The mean doubles from `12` to `24`, and the SD and SE also double;
the number of observations does not change.

**Exercise C:** Try the four readings `5,5,5,5`. Every deviation is zero.
The SD and the computed SE are both zero. This does not prove the larger
population has no variability: perhaps the sample was unrepresentative.

**Exercise D:** A sensor prints `5` four times because it reused one stored
reading. Is `n=4` a fair count of independent observations? No: those
repetitions are not new independent evidence.

## 8. What to learn next

The next useful topics are independent versus correlated measurements,
how the central limit theorem relates to sample means, and how a
Student-t confidence interval is constructed for small independent samples.
These require further assumptions; start with understanding the SE formula.

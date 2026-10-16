# Beginner's Guide: What Does "Return per Unit of Risk" Mean?

You can complete this lesson with elementary arithmetic and no finance
background. We will make an actual numerical calculation by hand, then
translate it into a small C++17 program. It continues
[Lesson 04: portfolio returns](../04-portfolio-returns/) and
[Lesson 05: portfolio risk](../05-two-asset-portfolio-risk/).
If the words *mean* or *standard deviation* are unfamiliar, their definitions
and the entire calculation appear below.

## 1. The question

Suppose two fictional investments both earn money **on average**. One has
large ups and downs; the other has small ones. Comparing only their average
returns misses something important: how much those returns fluctuate.

One commonly used historical summary is the **Sharpe ratio**, named after
William F. Sharpe. In this introductory lesson, it answers:

> How large was the observed average return **above a benchmark**, compared
> with the observed standard deviation of those returns?

It is a summary of a small **sample** of past or fictional numbers, not a
guarantee, prediction, or direct measurement of the chance of losing money.

## 2. Learn the vocabulary first

- An **asset** is something you can own, such as an investment.
- A **portfolio** is a collection of assets held together.
- A **return** describes how much an asset's value changed relative to its
  starting value: `(ending value - starting value) / starting value`.
- A **period** is one equal-length interval. We use *one month*.
- A **percentage** represents a fraction per hundred: `2% = 2/100 = 0.02`.
- A **decimal return** is a return written as a fraction, such as `0.02`.
  In this program, **all inputs use decimals**, never the number `2` for 2%.
- A **negative return** is a loss over that period: `-1% = -0.01`.
- A **weight** is the share of portfolio money allocated to an asset.
  For two assets, weights of 50% and 50% mean `0.5` and `0.5`.
- A **weighted average** multiplies each value by its weight and adds them.
- **Rebalancing** means restoring the intended weights before another period.
  We assume this happens monthly, without any costs.
- A **sample** is a limited set of observations. Here: five invented months.
- The **arithmetic mean** is the ordinary average: add values and divide
  by the number of values.
- A **benchmark** is a reference value against which another value is compared.
- A **risk-free rate** is an idealized benchmark return for the same period.
  In real life, its choice requires care; it is not a promise of perfect safety.
- An **excess return** is `investment return - benchmark return`.
- A **deviation** is the difference between one observed value and the mean.
- A **variance** measures squared deviations around a mean.
- A **standard deviation** is the square root of variance. It measures
  historical spread; it is **not** a complete definition of financial risk.
- A **sample standard deviation** uses the sample-variance denominator
  `number of observations - 1`.
- The **Sharpe ratio** divides average excess return by standard deviation.
  It has no units because both numerator and denominator have return units.

### A percentage-point distinction

A change from 2% to 5% is an increase of **3 percentage points**.
Those returns, as program inputs, are `0.02` and `0.05`; their difference
is `0.03`. We multiply by 100 **only for display**, to show 3 percentage points.

## 3. Start with two invented assets

We choose exactly two equally weighted assets, A and B. The weights at the
start of *every* month are `0.5` and `0.5`.

Our fictional observations are:

| Month | Asset A | Asset B | Portfolio calculation | Portfolio return |
| --- | ---: | ---: | --- | ---: |
| 1 | +4% | 0% | `0.5*0.04 + 0.5*0.00` | +2% |
| 2 | -2% | 0% | `0.5*(-0.02) + 0.5*0.00` | -1% |
| 3 | +6% | 0% | `0.5*0.06 + 0.5*0.00` | +3% |
| 4 | 0% | 0% | `0.5*0.00 + 0.5*0.00` | 0% |
| 5 | +1% | +1% | `0.5*0.01 + 0.5*0.01` | +1% |

The monthly **portfolio return series** is:

```text
+2%, -1%, +3%, 0%, +1%
```

In the C++ data format, that same series is:

```text
0.02, -0.01, 0.03, 0.00, 0.01
```

This is the same weighted-return idea used in Lesson 04 and Lesson 05.
Do not accidentally combine asset A's Monday result with asset B's Tuesday
result, or mix daily observations with monthly observations.

## 4. Calculate the ordinary average

We have five monthly returns, so `n = 5` (`n` means the count).

```text
mean = (2% + (-1%) + 3% + 0% + 1%) / 5
     = 5% / 5
     = 1% per month
```

In decimal form:

```text
mean = (0.02 - 0.01 + 0.03 + 0.00 + 0.01) / 5
     = 0.05 / 5
     = 0.01
```

So the **sample arithmetic mean** is `0.01`, or +1% per month.
This does *not* mean any coming month will produce exactly 1%.

## 5. Measure the spread with sample standard deviation

An **observation** is one monthly return. Subtract the mean from each:

| Month | Return | Mean | Deviation (return minus mean) |
| --- | ---: | ---: | ---: |
| 1 | +2% | +1% | +1 percentage point |
| 2 | -1% | +1% | -2 percentage points |
| 3 | +3% | +1% | +2 percentage points |
| 4 | 0% | +1% | -1 percentage point |
| 5 | +1% | +1% | 0 percentage points |

Now **square** each decimal deviation. Squaring means multiplying a number
by itself. Even a negative deviation produces a nonnegative square.

```text
( 0.01)^2 = 0.0001
(-0.02)^2 = 0.0004
( 0.02)^2 = 0.0004
(-0.01)^2 = 0.0001
( 0.00)^2 = 0.0000
                   ------
sum                0.0010
```

**Sample variance** divides this sum by `n - 1`, not `n`:

```text
sample variance = 0.0010 / (5 - 1)
                = 0.0010 / 4
                = 0.00025
```

Why `n - 1`? We first estimated the mean from the same five observations.
The common **sample** convention compensates for that estimated mean by
using one fewer degree of freedom. A degree of freedom is an independent
piece of information: once you know four deviations and know that they sum
to zero, the fifth is fixed. Dividing by `n` instead describes the
**population** variance of precisely these five values. Both conventions
exist, but we deliberately use the sample version here.

A **square root** reverses squaring; for example, `sqrt(9) = 3`.

```text
sample standard deviation = sqrt(0.00025)
                          = 0.015811388...
                          = 1.5811388... percentage points per month
```

A standard deviation measures the observed *spread of all returns*.
Negative returns, large losses, and unusual events can still happen; the
number alone does not describe every form of investment risk.

## 6. Introduce the matching-period benchmark

Imagine a fictional monthly **risk-free benchmark** of 0.2%. Write it in
C++ as `0.002`.

A **monthly benchmark must be compared with monthly returns**. Subtracting
an annual interest rate from a monthly mean would mix unlike units.
We assume this example's benchmark is a constant 0.2% *each* month.

For every month, the return above that benchmark would be:

| Month | Portfolio return | Benchmark | Excess return |
| --- | ---: | ---: | ---: |
| 1 | +2.0% | +0.2% | +1.8% |
| 2 | -1.0% | +0.2% | -1.2% |
| 3 | +3.0% | +0.2% | +2.8% |
| 4 | 0.0% | +0.2% | -0.2% |
| 5 | +1.0% | +0.2% | +0.8% |

The average excess return is:

```text
(1.8% - 1.2% + 2.8% - 0.2% + 0.8%) / 5
= 4.0% / 5
= 0.8% per month
= 0.008 as a decimal
```

Because the benchmark is **constant**, we may calculate the same answer
more simply:

```text
mean excess return = mean portfolio return - monthly benchmark
                   = 0.01 - 0.002
                   = 0.008
```

Subtracting a fixed number from every observation moves all returns by
the same amount and **does not change standard deviation**. A *varying*
benchmark would require calculating a separate series of excess returns;
that is beyond this first lesson.

## 7. Divide to get the sample Sharpe ratio

Our simple historical **sample Sharpe ratio** is:

```text
            mean portfolio return - risk-free return
Sharpe  =   ----------------------------------------
                    sample standard deviation
```

Insert the *decimal* numbers, not percent labels:

```text
Sharpe = (0.01 - 0.002) / sqrt(0.00025)
       = 0.008 / 0.015811388...
       = 0.505964425...
       ≈ 0.506
```

**Interpretation:** in these five fictional months, the sample mean
excess return was approximately 0.506 times the sample standard deviation.
That does not mean a 50.6% chance of profit or a 50.6% return! The ratio
has **no percent sign**.

The number refers to the observed *monthly* data. We do not multiply it
by `sqrt(12)` to produce an annualized ratio, because doing so introduces
additional assumptions about the return process and the benchmark.
There is no claim about future investment performance.

## 8. Two edge cases you must understand

**Case A: average return equals benchmark.**
If the sample mean is 2% and the monthly benchmark is also 2%, average
excess return is zero. With nonzero standard deviation, Sharpe is zero.

**Case B: every return is identical.**
If every month is 1%, deviations are all zero, so standard deviation is
zero. The expression `excess / zero` is **undefined**, even if excess is
also zero. The program reports an error rather than printing infinity or
an invented Sharpe ratio.

A negative Sharpe ratio is possible when average observed return is below
the matching benchmark. That is a description of the sample, not a
prediction.

## 9. Read the C++ program, step by step

### The data types and header

Open `sharpe.hpp`. A **header file** makes code available to multiple
C++ source files, here `main.cpp` and `tests.cpp`.

- `#include <vector>` supplies `std::vector`, a list that can grow.
- `#include <cstddef>` supplies `std::size_t`, a type for counting items.
- `#include <cmath>` supplies `std::sqrt` and `std::isfinite`.
- `#include <stdexcept>` supplies standard exception classes.
- A `namespace` is a named group of code. `lesson06::` prevents name
  collisions with unrelated functions in another lesson.
- `double` is a floating-point type: it stores approximate real numbers.
  Most decimals cannot be represented exactly in binary.
- `long double` is another floating-point type used for intermediate
  sums and squared deviations. It may offer more precision on a given
  compiler, but it cannot eliminate all rounding or overflow problems.
- `struct SharpeSummary` groups several named results together.
- `[[nodiscard]]` encourages the caller to use a returned result.
- `inline` lets a function definition live in a header included by
  several source files without creating a duplicate-definition error.
- A `function` is named code that accepts **arguments** and returns a
  result. The **return type** says what kind of result it produces.

### Combining assets

The function `two_asset_returns(asset_a, asset_b, weight_a)` is a
small extension of the previous lesson:

1. `const std::vector<double>&` passes an existing list by **reference**
   (no copy); `const` says the function does not change it.
2. `asset_a.size()` tells us how many monthly observations exist.
3. `if` checks conditions. The first check rejects fewer than two
   observations or unmatched asset-list sizes.
4. `std::isfinite` checks that a value is neither an infinity nor
   `NaN` (*not a number*).
5. A weight outside `[0,1]` is rejected. This example deliberately
   forbids **leverage** (borrowing to increase exposure) and short positions.
6. `1.0 - weight_a` computes the second asset's weight.
7. `std::vector<double> result` creates an initially empty list.
8. `result.reserve(...)` reserves enough storage in advance.
9. `for (std::size_t i = 0; i < asset_a.size(); ++i)` repeats the block,
   starting at index 0. `++i` increases the index by one each time.
10. `asset_a[i]` retrieves one observed value; square brackets index
    the list starting at zero.
11. `weight_a * asset_a[i] + weight_b * asset_b[i]` is our weighted return.
12. `result.push_back(monthly)` appends that value; `return result`
    gives the complete new vector back to the caller.

`throw std::invalid_argument(...)` is how this function reports a bad
input. An **exception** interrupts normal execution so that a caller can
handle the problem. We do not silently fix wrong dates or weights.

### Computing Sharpe

Now follow `analyze_sharpe(period_returns, risk_free_per_period)`:

1. The first `if` requires at least two observations. This is needed
   because sample variance divides by `n-1`.
2. The second `if` verifies a finite benchmark rate.
3. `long double sum = 0.0L` starts an accumulation; the suffix `L`
   selects a `long double` literal.
4. The first `for (double value : period_returns)` is a
   **range-based for loop**: it visits each number in the list.
5. `sum += static_cast<long double>(value)` means
   `sum = sum + value`, converting the input to the accumulator type.
6. `value != first` checks whether there is any observed variation.
   A constant series has an undefined Sharpe denominator.
7. `n = period_returns.size()` counts observations. `mean = sum / n`
   calculates the arithmetic average in the same decimal units.
8. The second loop computes `difference = value - mean`, squares it
   with `difference * difference`, and adds the squares.
9. Dividing by `n-1` produces the sample variance.
10. `std::sqrt(variance)` produces the sample standard deviation.
11. `excess = mean - risk_free_per_period` computes mean excess return.
12. `ratio = excess / standard_deviation` gives the sample Sharpe ratio.
13. A `SharpeSummary` stores the count and six named numbers so callers
    do not confuse variance, standard deviation, and Sharpe.
14. `static_cast<double>` converts intermediate results back to the
    public return type. Additional checks reject nonfinite results.
15. `std::domain_error` reports an undefined numerical operation, such
    as the zero-volatility case.

The two-pass calculation above is deliberately transparent: one pass
finds the mean; the second measures squared deviations. A future numerical
methods lesson can compare alternative formulas and their accuracy.

### Printing the results

In `main.cpp`:

- `const std::vector<double> asset_a{...}` creates an immutable list
  initialized with five invented monthly returns.
- `const auto portfolio = ...` lets the compiler deduce the returned
  type, here a vector. `auto` is not an untyped variable.
- `std::cout` writes text to the terminal.
- `std::fixed` requests fixed-point formatting.
- `std::setprecision(3)` prints three digits after the decimal point.
- We multiply mean, benchmark, and standard deviation by `100` to
  display percentages. **We do not multiply the Sharpe ratio by 100.**
- `'\n'` represents a new line.
- When `main` reaches its end, C++ automatically returns success code 0.

## 10. Compile the program and run actual checks

A **compiler** translates C++ source code into an executable program.
From the directory containing this lesson, run:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o sharpe_demo
./sharpe_demo
```

- `g++` invokes the GNU C++ compiler.
- `-std=c++17` selects the C++17 language rules.
- `-O2` requests compiler optimizations.
- `-Wall -Wextra -Wpedantic` request useful warnings.
- `-Werror` makes warnings fail compilation.
- `-o sharpe_demo` sets the resulting executable's name.
- `./sharpe_demo` runs that program on a Unix-like shell.

Compare the numbers with the calculations in Sections 4 to 7. They should
be printed to three decimal places, so `0.505964...` appears as `0.506`.

Now compile and run the **unit tests**. A unit test is an automated check
that a small part of a program behaves as expected.

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o sharpe_tests
./sharpe_tests
```

The tests check specific hand-calculated numbers, a negative ratio,
the zero-excess case, wrong input sizes, invalid weights, nonfinite
inputs, and the division-by-zero case. A successful compilation is
*not* sufficient evidence of correct arithmetic; that is why we run tests.

## 11. Common mistakes and limitations

1. **Confusing units.** Writing `2` for 2% multiplies your intended
   return by 100. Use `0.02`.
2. **Mixing periods.** A monthly standard deviation and yearly benchmark
   must not be combined directly.
3. **Confusing variance and standard deviation.** Variance is in squared
   return units. Standard deviation uses the original return units.
4. **Treating a negative Sharpe as an error.** It can be a valid result;
   a zero standard deviation, however, makes the ratio undefined.
5. **Using constant portfolio weights without understanding rebalancing.**
   With no rebalancing, weights drift when assets grow at different rates.
6. **Assuming Gaussian (bell-shaped) returns.** The calculation does not
   require returns to follow a normal distribution. Interpretation of
   unusual large losses still matters.
7. **Believing a historical Sharpe is a prediction.** Five observations
   offer very little statistical confidence; even long samples can fail
   to predict future outcomes.
8. **Ignoring changing benchmarks, costs, or compounding.** This simple
   calculation includes none of them.
9. **Comparing ratios from different conventions.** An annualized
   population-variance result and a monthly sample-variance result are
   not automatically comparable.
10. **Over-interpreting volatility.** Standard deviation treats unusually
    positive and negative returns as variation; it is not a measure of
    liquidity, drawdown, tail loss, or how much capital you can afford to lose.

This guide makes no claim that the fictional example corresponds to a
real investment, trading system, or achievable performance.

## 12. Exercises with answers

**Exercise A — change the benchmark.**
Keep the original five monthly returns but change the monthly benchmark
from 0.2% to 1%. What happens?

Answer: Mean excess return becomes `1% - 1% = 0%`. Standard deviation
remains about `1.581%`. Sharpe becomes `0 / 0.015811... = 0`.

**Exercise B — check signs.**
Use two months of +3% and +1%, with a benchmark of +3%. Is Sharpe positive?

Answer: The mean is 2%; mean excess return is -1%. Sample standard
deviation is `sqrt(0.0002) = 0.014142...`. Sharpe is about `-0.707`.

**Exercise C — identical returns.**
What does the function do with `[0.01, 0.01, 0.01]`?

Answer: Sample standard deviation is zero. The ratio is undefined.
It throws `std::domain_error`; it does not return zero.

**Exercise D — read the code.**
Change the weight of asset A from 50% to 100%. What series results?

Answer: The combined series becomes the asset A series
`[0.04, -0.02, 0.06, 0.00, 0.01]`.
The weight of B is zero. Recalculate mean and spread before interpreting.

**Exercise E — conceptual.**
Why can two portfolios have the same average return but different
Sharpe ratios?

Answer: Their observed standard deviations can differ, even when their
mean excess returns and matching benchmark are identical.

**Exercise F — stretch question.**
Could you compare the ratio here with a daily Sharpe ratio of 0.4 and
immediately conclude that one investment is better?

Answer: No. Their periods, benchmark conventions, volatility estimates,
and data quality differ. Consistent methodology is needed before comparison.

## 13. Where to go next

You now know how to transform period-by-period portfolio returns into a
historical *risk-adjusted* summary. Next, a separate lesson can explore
rolling windows: recomputing the mean and standard deviation over a moving
subset of months. Later topics include varying benchmark series, different
volatility estimators, downside risk, and numerical stability.

The important habit is to define assumptions, show the units, check
invalid cases, and test the code before interpreting a financial number.

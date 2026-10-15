# Beginner's Guide: Portfolio Risk from First Principles

You do not need previous investment experience. You should know basic addition, multiplication, and division. This lesson continues the [portfolio return](../04-portfolio-returns/BEGINNERS_GUIDE.md) lesson and uses the idea of [covariance](../../statistics/02-covariance-correlation/).

## 1. The question we want to answer

Imagine investing 60% of your savings in asset A and 40% in asset B. Your investments change value each month. Their **average return** might be positive even if some months are poor. We want to measure how widely your portfolio's observed returns vary around their average.

Definitions:

- An **asset** is something owned that has value.
- A **portfolio** is a collection of assets.
- A **return** is the relative change in value during a stated period: `(end - start) / start`.
- A **period** is one observation interval (a month in our example).
- A **weight** is the fraction of money assigned to an asset. Two weights of 0.60 and 0.40 sum to 1.
- **Rebalancing** means resetting a portfolio to its chosen weights. We assume this happens before each month in our simplified calculation.
- A **sample** is a finite set of observed values. We use three fictional monthly observations.
- **Risk**, in everyday language, means that actual outcomes can differ from what you expect. Standard deviation measures only one aspect of that: observed variability.

A percentage must be converted to a decimal before calculation: `10% = 0.10`; `-2% = -0.02`.

## 2. Start with a single month's portfolio return

If asset A returns 10% and asset B returns 0%, the portfolio return with weights 60% and 40% is:

`0.60 * 0.10 + 0.40 * 0.00 = 0.06 = 6%`.

This is a **weighted average**: each asset contributes according to the money allocated to it. The weights describe the *starting allocation for that month*.

## 3. Three months of fictional data

Every row uses returns for the same month in both assets.

| Month | A return | B return | 60% of A + 40% of B |
| --- | ---: | ---: | ---: |
| 1 | +10% | 0% | `0.60*0.10+0.40*0.00 = 0.060` = +6.0% |
| 2 | -2% | +4% | `0.60*(-0.02)+0.40*0.04 = 0.004` = +0.4% |
| 3 | +6% | +2% | `0.60*0.06+0.40*0.02 = 0.044` = +4.4% |

The portfolio return series is therefore `[0.060, 0.004, 0.044]`. Its **mean** (ordinary arithmetic average) is:

`(0.060 + 0.004 + 0.044) / 3 = 0.036 = 3.6% per month`.

The mean is a description of these three observations, not a prediction of a future month. Do not multiply it by 12 and call the result a guaranteed annual return.

## 4. Variance: how far do returns move from the mean?

A **deviation** is `observed return - mean`. Our portfolio deviations are:

- Month 1: `0.060 - 0.036 = +0.024`.
- Month 2: `0.004 - 0.036 = -0.032`.
- Month 3: `0.044 - 0.036 = +0.008`.

We square them so positive and negative values do not cancel:

- `0.024 * 0.024 = 0.000576`.
- `(-0.032) * (-0.032) = 0.001024`.
- `0.008 * 0.008 = 0.000064`.

The total is `0.001664`. **Sample variance** is the sum of squared deviations divided by `n - 1`, where `n` is the number of observations. Here, `n=3`, so:

`sample variance = 0.001664 / (3 - 1) = 0.000832`.

Why `n-1`? We estimated the mean from the same data. Dividing by `n-1` is the standard sample-variance convention; dividing by `n` instead gives the **population variance** of these exact three values. These are different questions, so do not mix the two conventions.

**Standard deviation** is the nonnegative square root of variance. A square root is the number which, multiplied by itself, gives the original number:

`standard deviation = sqrt(0.000832) = 0.028844... = 2.8844... percentage points`.

Because the input returns are decimals, multiply by 100 only when displaying the result as a percent. The standard deviation is measured in the same return units as the observations.

## 5. Why having two assets changes variability

**Covariance** records whether the deviations of two series tend to point in the same direction. To calculate it, multiply A's and B's deviations in each month, sum the products, and divide by `n-1`.

For this example:

- Mean A = `(0.10 - 0.02 + 0.06)/3 = 0.046666...`.
- Mean B = `(0 + 0.04 + 0.02)/3 = 0.02`.
- A deviations = `[+0.053333..., -0.066666..., +0.013333...]`.
- B deviations = `[-0.02, +0.02, 0]`.
- Products = `[-0.00106666..., -0.00133333..., 0]`.
- Sample covariance = `(-0.0024) / 2 = -0.0012`.

A negative covariance says these *observed* series tended to move in opposite directions relative to their respective means. It does not promise that they will offset each other next month.

The individual sample variances are:

`variance(A) = 0.003733333...` and `variance(B) = 0.0004`.

There is an alternative way to calculate portfolio variance, using the **two-asset variance identity**:

`variance(portfolio) = wA^2 * variance(A) + wB^2 * variance(B) + 2*wA*wB*covariance(A,B)`.

Here, `wA = 0.60`, `wB = 0.40`, and `wA^2` means `wA * wA`:

`0.36*0.003733333... + 0.16*0.0004 + 0.48*(-0.0012)`

`= 0.001344 + 0.000064 - 0.000576 = 0.000832`.

This matches the variance computed directly from the monthly portfolio returns. The negative covariance reduces the combined variance in this example. This is one way to understand **diversification**: combining assets can change variability rather than merely averaging their individual risks.

## 6. Translate the mathematics into C++

C++ is a programming language. We use the C++17 version of its language rules and standard library. Open `risk.hpp` while reading this section.

- `#include <vector>` makes `std::vector` available. A **vector** is a list of elements; here each element is a monthly return.
- `double` stores a floating-point number, which is an approximation of a decimal value. It is appropriate for this small educational calculation, but rounding still exists.
- `std::size_t` is an unsigned integer type used for counting elements and indexing vectors.
- `struct RiskSummary` groups several computed numbers into one named result.
- `analyze_portfolio(...)` is a **function**: it takes arguments (two lists and a weight) and returns a `RiskSummary`.
- `const std::vector<double>&` lets the function read a list without copying or modifying it. The `&` is a **reference**; `const` means read-only.
- `[[nodiscard]]` asks the compiler to warn when somebody ignores the function's returned result.
- An `if` statement checks a condition; `throw std::invalid_argument(...)` rejects invalid input rather than producing a misleading number.
- `std::isfinite` checks that a number is neither infinity nor **NaN** ("not a number").
- `for (std::size_t i = 0; i < count; ++i)` repeats a block of code, starting at index 0 and stopping before `count`. `++i` means increase `i` by one.
- `returns_a[i]` accesses item `i` in the list; the first item has index 0.
- `portfolio_returns.reserve(count)` asks the vector to reserve space for `count` items; `push_back(p)` then adds each computed return.
- `sum_a += a` is short for `sum_a = sum_a + a`.
- `static_cast<double>(count)` converts a count into the numeric type used for division.
- Each `deviation` subtracts the relevant mean; multiplying a deviation by itself gives its square.
- `std::sqrt` finds the square root. The function returns all computed values inside `RiskSummary`.
- `inline` allows this small function definition to live in a header included by more than one C++ source file without violating the one-definition rule.

Open `main.cpp`: we pass the same three data rows as our table to the function and print the mean and standard deviation multiplied by 100, because users prefer percentage displays. Open `tests.cpp`: it checks known arithmetic outcomes and verifies that invalid inputs throw exceptions. An **exception** is a way of reporting a problem to the calling code. `std::runtime_error` makes a failed test stop with an explanatory message.

## 7. Compile, run, and interpret the output

A **compiler** translates C++ source code into an executable program. In the lesson directory run:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o portfolio_risk
./portfolio_risk
```

`-std=c++17` selects the language version. `-O2` requests optimizations. The `-W...` options request warnings; `-Werror` treats warnings as errors. `-o portfolio_risk` selects the executable's filename. `./portfolio_risk` runs the built program on Unix-like systems.

Expected output:

```text
Mean portfolio return: 3.600%
Sample portfolio standard deviation: 2.884%
Sample covariance (A,B): -0.001200
```

Now compile and run `tests.cpp` using the same flags and replace `main.cpp` with `tests.cpp`, then choose `-o portfolio_risk_tests`. Tests require known answers because compilation alone cannot prove a calculation is correct.

## 8. Common mistakes, limitations, and exercises

- **Percent versus decimal:** Enter `0.10` for +10%, not `10`.
- **Mismatched dates:** Covariance is meaningful only for returns measured over the same periods.
- **Too few observations:** With one observation, sample variance cannot divide by `n-1`.
- **Different rebalancing:** If weights drift with asset prices, your monthly portfolio returns need a different model.
- **False certainty:** Historical standard deviation alone does not measure tail losses, liquidity risk, or future uncertainty.
- **Extreme values:** Very large finite floating-point inputs can overflow; this implementation rejects nonfinite results.

Try these before looking at the tests:

1. **Weights:** Set `weight_a` to 1.0. Which asset determines the portfolio variance? **Answer:** Only A; the portfolio is all A.
2. **Cancellation:** Use A returns `[+0.10, -0.10]`, B returns `[-0.10, +0.10]`, and equal weights. **Answer:** Both portfolio returns equal zero, so sample standard deviation is zero.
3. **Unit conversion:** If variance is `0.0004`, what is standard deviation in percent? **Answer:** `sqrt(0.0004)=0.02 = 2%`.
4. **Conceptual:** Why is negative covariance useful here? **Answer:** In the sample, it reduces portfolio variance by offsetting some deviations; this is not a guarantee of future performance.

## 9. What to study next

With portfolio returns, sample variance, and covariance established, you can compare portfolios under different fixed weights. Later lessons can introduce correlation, rolling time windows, uncertainty estimates, and more realistic portfolio constraints. None of these introductory calculations is a trading strategy or a claim of investment performance.

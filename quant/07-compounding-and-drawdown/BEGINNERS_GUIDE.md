# Beginner's Guide: From Monthly Returns to Maximum Drawdown

This is Lesson 07 in the C++ quantitative finance track. You do **not** need
prior experience with stocks, trading, or statistics. If you can multiply two
numbers, you already know the main mathematical operation we need. Previous
lessons introduced portfolio returns and measures of variability. We now ask
a different question: **how large was the worst fall from a past high?**

## 1. Words to learn, one at a time

**Portfolio:** A collection of investments treated as one whole. Here our
portfolio is a fictional amount of wealth; we do not have to model individual
stocks.

**Wealth:** The portfolio's value at a particular time. We start at 100
imaginary monetary units, so no real price or currency is involved.

**Period:** One step in time. All five steps in our example are one month
long. Period 0 is the moment before the first monthly change.

**Return:** A change relative to the earlier value. For example, growing from
100 to 120 means `(120 - 100) / 100 = 0.20 = +20%`. Falling from 120 to 90
means `(90 - 120) / 120 = -0.25 = -25%`.

**Decimal fraction:** The representation used in calculations: `0.20` means
20%, *not* 0.20%. To turn a decimal into a percentage, multiply by 100.

**Return multiplier:** `1 + return`. A +20% return has multiplier `1.20`;
a -25% return has multiplier `0.75`. We multiply wealth by this factor.

**Compounding:** Applying each new multiplier to the wealth that remains
*after* all previous periods. Both profits and losses affect later results.

**Cumulative return:** The total percentage change from the beginning to the
end. It is `(final wealth / starting wealth) - 1`, not the sum of monthly
percentage changes.

**Running peak / high-water mark:** The largest wealth value observed up to
and including the current period. We cannot use a peak that happens later.

**Drawdown:** The fraction that today's wealth is below its running peak:
`(running peak - current wealth) / running peak`. It is zero at a new high.
It is nonnegative; a 25% drawdown appears as `0.25` in calculations.

**Maximum drawdown:** The largest drawdown across the observed periods.
This is not the biggest one-month negative return in general; a fall can
continue for several months.

**Peak period and trough period:** The earlier high and later low responsible
for the worst decline. A *trough* is a low point. We identify them using
zero-based periods: the beginning is period 0.

**Historical:** Computed from observations already supplied. It does not
predict a future loss.

## 2. Build the wealth table by hand

Start with wealth 100 and the following *made-up* monthly returns:

```text
Period 1: +20%  -> multiplier 1.20
Period 2: -25%  -> multiplier 0.75
Period 3: +50%  -> multiplier 1.50
Period 4: -20%  -> multiplier 0.80
Period 5: +10%  -> multiplier 1.10
```

Now multiply **the result from the previous line**, never the original 100
again:

```text
Beginning  = 100.00
After month 1 = 100.00 * 1.20 = 120.00
After month 2 = 120.00 * 0.75 =  90.00
After month 3 =  90.00 * 1.50 = 135.00
After month 4 = 135.00 * 0.80 = 108.00
After month 5 = 108.00 * 1.10 = 118.80
```

The final wealth is 118.80. Therefore:

```text
Cumulative return = 118.80 / 100.00 - 1
                  = 1.188 - 1
                  = 0.188
                  = +18.8%
```

### Why not add the percentages?

Adding `20% - 25% + 50% - 20% + 10%` gives `+35%`, which is **not** the actual
change in wealth. The arithmetic mean of these five values is `35% / 5 = 7%`
per month. It can describe the observations, but five months of 7% does not
mean that the portfolio earned 35% cumulatively.

A tiny example makes the reason obvious: a 50% loss turns 100 into 50; a 50%
gain then turns 50 into 75. The returns add to zero but wealth is **25% lower**.
The base of the percentage changes from one month to the next.

## 3. Calculate each running peak

A running peak remembers the best value seen *so far*. Include the 100 at
period 0. After each new wealth value, update the peak if it is higher:

| Period | Wealth | Peak before or at this period |
| ---: | ---: | ---: |
| 0 | 100.00 | 100.00 |
| 1 | 120.00 | 120.00 |
| 2 | 90.00 | 120.00 |
| 3 | 135.00 | 135.00 |
| 4 | 108.00 | 135.00 |
| 5 | 118.80 | 135.00 |

Note that **135 at period 3 cannot be used when measuring period 2**.
At period 2, 135 has not happened yet. Using future information in a past
calculation is sometimes called *look-ahead bias*; the simple rule of a
running peak avoids that error.

## 4. Calculate drawdowns from these peaks

Use `(peak - wealth) / peak` separately at each period:

```text
Period 0: (100 - 100) / 100 = 0.00 = 0%
Period 1: (120 - 120) / 120 = 0.00 = 0%
Period 2: (120 -  90) / 120 = 0.25 = 25%
Period 3: (135 - 135) / 135 = 0.00 = 0%
Period 4: (135 - 108) / 135 = 0.20 = 20%
Period 5: (135 - 118.80) / 135 = 0.12 = 12%
```

The **largest** value is 25%, observed at period 2. The earlier associated
peak was 120 at period 1. We say the historical maximum drawdown is 25%,
from period 1 to period 2. Even though period 3 reaches a new high, that
does not erase the earlier drawdown.

### The worst one-month return is not always the maximum drawdown

Imagine 100, then -10% to 90, then another -10% to 81. Both monthly returns
were -10%, but the fall from the original peak is `(100 - 81)/100 = 19%`.
Maximum drawdown is therefore **19%**, not 10%.

## 5. The smallest useful C++ program

A **vector** is a C++ container that can hold a sequence of values. We use
`std::vector<double>`: `double` stores numbers that can have decimal parts.
A vector keeps the months in chronological order.

```cpp
#include "drawdown.hpp"
#include <vector>

int main() {
    const std::vector<double> monthly_returns{0.20, -0.25, 0.50, -0.20, 0.10};
    const auto result = quant::analyze_drawdowns(monthly_returns, 100.0);
    return result.maximum_drawdown >= 0.0 ? 0 : 1;
}
```

- `#include "drawdown.hpp"` makes our function and data types available.
  A **header** is a file that gives C++ definitions needed by other files.
- `#include <vector>` provides the standard-library vector container.
- `int main()` is where a C++ program starts.
- `const` means we do not intend to modify this vector after creating it.
- `{...}` gives its five values, in order.
- `auto` asks the compiler to deduce the type returned by the function.
- `quant::` is a **namespace**, a way to group names so they do not collide
  with unrelated code.
- `analyze_drawdowns(monthly_returns, 100.0)` asks for the period-by-period
  table and summary, using initial wealth 100.
- `.maximum_drawdown` accesses one named field of the returned summary.
- The `return` statement is this tiny program's exit status. A status of 0
  normally indicates success. The full `main.cpp` prints useful values.

## 6. Read the algorithm in `drawdown.hpp`

We separate the reusable calculation from screen output. `main.cpp` is only
one example; another program can include `drawdown.hpp` to analyze its own
validated sequence.

### Important data types

```cpp
struct PeriodResult {
    std::size_t period;
    double period_return;
    double wealth;
    double running_peak;
    double drawdown;
};
```

A **struct** groups related fields under one name. Each `PeriodResult` is a
row of the table. `std::size_t` is an unsigned integer type used for sizes
and indexes. In our code, periods start at 0.

`DrawdownSummary` contains a `std::vector<PeriodResult>` called `path` plus
the cumulative return, maximum drawdown, and the peak/trough period indexes.
That gives calling code both the **individual steps** and the **summary**.

### Parameters and their validation

```cpp
[[nodiscard]] inline DrawdownSummary analyze_drawdowns(
    const std::vector<double>& returns,
    double initial_wealth = 100.0)
```

- A **function** takes inputs (parameters) and returns a result.
- `const ...&` is a **const reference**: we can read the caller's vector
  without copying it or changing it.
- `= 100.0` gives the starting wealth a **default value**.
- `[[nodiscard]]` asks the compiler to warn if a caller discards the result.
- `inline` allows this small function to be defined in a header included
  in multiple translation units. A *translation unit* is one source file
  after its includes have been processed; `inline` is not a speed guarantee.

The function rejects an empty sequence, a nonpositive or nonfinite starting
wealth, and returns below -100%. It uses `std::isfinite` to reject `NaN`
(*not a number*) or infinity. These are special floating-point values that
would make a reported percentage misleading or undefined.

A **C++ exception** reports an error to the caller. This function uses
`std::invalid_argument` for impossible inputs and `std::overflow_error` if
an intermediate wealth or final result cannot fit in a `double`. A caller
can catch those exception types to handle bad inputs. The demo supplies
valid constants, so no exception occurs there.

### The update loop, translated into English

```cpp
for (std::size_t i = 0; i < returns.size(); ++i) {
    const double period_return = returns[i];
    // Validate this return first.
    wealth *= 1.0 + period_return;
    // Reject unrepresentable wealth.
    if (wealth > running_peak) {
        running_peak = wealth;
        current_peak_period = i + 1;
    }
    const double drawdown = std::clamp(
        (running_peak - wealth) / running_peak, 0.0, 1.0);
    // Store a row and update maximum drawdown when necessary.
}
```

- `for` repeats an operation once for each return.
- `i = 0` starts at the vector's first element; `++i` increments the index.
- `returns.size()` is the number of observations.
- `returns[i]` reads the current month's return.
- `wealth *= factor` is shorthand for `wealth = wealth * factor`.
- `if` chooses actions only when its condition is true.
- `std::clamp(x, 0.0, 1.0)` keeps a calculated drawdown within 0 and 1.
  The mathematically valid range is already 0 to 1 here; clamping also
  protects the displayed result from tiny roundoff at the boundaries.
- `i + 1` converts vector index zero into period one, because period zero
  already represents the starting wealth.

We update the running peak **before** computing the drawdown. Otherwise a
new record high might incorrectly appear as a negative drawdown. We keep a
separate index for the current peak, so we know where a later decline began.

When a drawdown strictly exceeds the previous maximum, the code remembers
its peak and trough. Strictly greater (`>`) means equal worst declines do
not replace the earliest one. If all periods reach new highs, the maximum
is zero and both recorded indexes remain zero by convention.

### Why `double` and why not compare for exact equality?

A `double` is a finite-precision floating-point number, not an exact decimal
calculator. Some decimal fractions have no exact binary representation.
For example, a program may internally store a value near 118.80 rather
than exactly 118.80. The demo rounds *display only* with `std::setprecision`.
The tests compare numeric results using a small tolerance, rather than
requiring exact equality between independently computed decimal values.

## 7. Run the example and its tests

Install a compiler supporting C++17 (for example, GCC or Clang). Open a
terminal in this lesson's directory and enter these commands one at a time:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o drawdown_demo
./drawdown_demo
```

The first command **compiles** (translates the C++ source into a runnable
program). `-std=c++17` chooses the C++ language version. `-O2` enables
common optimizations. The `-W...` options enable warnings, and `-Werror`
treats warnings as errors. `-o` gives the output file a name. The second
command executes that program.

Then test the calculation separately:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o drawdown_tests
./drawdown_tests
```

The test program checks known numerical answers and invalid inputs. A
**test** is code that compares actual behavior with an expected outcome.
A failure exits with a nonzero code; success prints `PASS: 50 checks`.

As an alternative, the `CMakeLists.txt` file tells **CMake** how to build
the two programs. **CTest** is CMake's test runner. See the exact four
commands in [README.md](README.md).

## 8. Four frequent beginner mistakes

1. **Entering `20` instead of `0.20`:** The code would interpret that as
   +2,000%, not +20%.
2. **Adding monthly returns:** Compounding multiplies the period factors;
   addition does not model the changing base.
3. **Using the final highest value as every period's peak:** A peak from
   the future must not affect an earlier drawdown.
4. **Confusing a return with a drawdown:** A -10% monthly change can
   produce a larger total drawdown if the portfolio was already below
   a previous peak.

## 9. Try these exercises before checking the answers

**Exercise A.** Wealth is initially 100. The returns are +10%, then -10%.
Find the final wealth, cumulative return, and maximum drawdown.

**Exercise B.** Wealth is initially 200. The only observed return is -30%.
Which period is the earlier peak? What is the trough period?

**Exercise C.** Wealth is initially 100 and rises by +5% twice. Can the
maximum drawdown be negative? What is the final wealth?

**Exercise D (small C++ edit).** Replace the five demonstration returns with
`{-0.10, -0.10}`. Compile and run again. Predict the maximum drawdown before
looking at the output. Do not change `drawdown.hpp`.

### Answers

**A:** `100 * 1.10 * 0.90 = 99`; cumulative return = -1%. Peak 110 after
period 1; period 2 wealth is 99, so drawdown is `(110-99)/110 = 10%`.
The maximum drawdown is 10% (even though the total change is only -1%).

**B:** Final wealth = `200 * 0.70 = 140`. Peak period = 0 and trough period
= 1. Maximum drawdown = 30%.

**C:** No. A drawdown is never negative in this definition. Final wealth =
`100 * 1.05 * 1.05 = 110.25`, and maximum drawdown = 0%.

**D:** Wealth follows `100 -> 90 -> 81`. Peak stays at period 0. The
maximum drawdown is `19%`, not `10%`, and trough period is 2.

## 10. What this number does **not** tell us

A single maximum drawdown does not say how often losses happened, how long
a recovery took, whether losses were caused by market movements, or what
could happen tomorrow. Monthly snapshots can hide deep intramonth losses.
A longer observation window is more likely to contain a large historical
fall, even if the underlying process is unchanged. Contributions,
withdrawals, leverage, fees, inflation, and cash-flow timing can also alter
real-world calculations. We deliberately leave all of those out here.

The numbers in this lesson are synthetic. The purpose is to understand the
math, read a tested C++17 implementation, and ask better questions about
historical risk — **not** to make trading decisions or promise performance.

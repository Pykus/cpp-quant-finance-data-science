# Lesson 06: Comparing Return and Variability with the Sharpe Ratio

**Who this is for:** A first-year student with no trading experience. We use five
fictional monthly observations and C++17. No live data, account, or package is needed.

**Start with [the beginner's guide](BEGINNERS_GUIDE.md).** It defines every new
term, walks through every arithmetic step, explains the source code, and gives
small exercises with answers. This lesson follows [portfolio returns](../04-portfolio-returns/)
and [two-asset portfolio risk](../05-two-asset-portfolio-risk/).

## What you will learn

- Combine two assets into a portfolio with fixed starting weights each month.
- Calculate arithmetic mean return and **sample** standard deviation.
- Subtract a matching-period risk-free benchmark return.
- Calculate the **sample Sharpe ratio**: mean excess return divided by sample
  standard deviation, and recognize when division is undefined.
- Explain why a historical ratio is **not** a forecast or a complete risk measure.

## Run with a C++17 compiler

In this directory:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o sharpe_demo
./sharpe_demo
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o sharpe_tests
./sharpe_tests
```

Alternatively, with CMake 3.16 or newer:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

No third-party libraries are used.

## Expected example output

```text
Observations: 5
Mean monthly return: 1.000%
Sample monthly standard deviation: 1.581%
Monthly risk-free rate: 0.200%
Mean monthly excess return: 0.800%
Sample monthly Sharpe ratio: 0.506
```

The result is dimensionless (a ratio), **per monthly period**. It has **not**
been annualized. Its particular numerical value comes entirely from invented
returns; it says nothing about investment opportunities.

## Files

- `sharpe.hpp`: two self-contained functions and the result structure.
- `main.cpp`: readable, runnable example using fabricated observations.
- `tests.cpp`: independently calculated reference values, boundary cases,
  invalid inputs, and zero-volatility behavior.
- `BEGINNERS_GUIDE.md`: mathematical derivation, code walkthrough,
  compilation steps, assumptions, interpretation, and exercises.
- `CMakeLists.txt`: optional repeatable build and test registration.

## Inputs and assumptions

All returns are **decimal fractions**: `0.02` is +2%, and `-0.01` is -1%.
The input asset observations must cover the same equally spaced months. The
portfolio uses a constant starting allocation, equivalent to rebalancing to
that allocation before every month. The benchmark is assumed constant over
the five matching months. There must be at least two observations with
nonzero observed variation, because otherwise the sample ratio is undefined.
All inputs and computed summary numbers must be finite.

These calculations ignore fees, taxes, compounding differences, liquidity,
drawdowns, estimation error, and changing benchmark rates. Historical
volatility captures only one kind of risk; a higher historical Sharpe ratio
does not guarantee any future outcome. Educational only, not investment advice.

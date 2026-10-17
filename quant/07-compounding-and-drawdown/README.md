# Lesson 07: Compounding Returns and Measuring Maximum Drawdown

**Audience:** A first-year student with no finance background and basic C++17
knowledge. **Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md).** It explains
all terminology, every numerical step, key code lines, input validation, and
exercises. This lesson follows [portfolio returns](../04-portfolio-returns/),
[two-asset portfolio risk](../05-two-asset-portfolio-risk/), and
[the historical Sharpe ratio](../06-sharpe-ratio/).

## Goal

A positive average return does **not** mean an investment grew smoothly. Here
we follow a fictional portfolio through five months, calculating its compounded
wealth and how far it fell from its previous best value.

By the end, you can:

- Turn a monthly return into a multiplier and compound consecutive returns.
- Explain the difference between adding returns and compounding them.
- Track a **running peak** (the greatest wealth observed so far).
- Calculate a **drawdown** (the decline from the running peak).
- Identify **maximum drawdown** and the peak/trough periods responsible.
- Recognize limitations of historical drawdown as a measure of risk.

## Numerical example

Start with **100** fictional monetary units. The returns in decimal form are
`{0.20, -0.25, 0.50, -0.20, 0.10}`. They mean +20%, -25%, +50%, -20%, +10%.
At every step: `new wealth = old wealth * (1 + period return)`.

| Period | Return | Wealth | Best wealth so far | Drawdown |
| ---: | ---: | ---: | ---: | ---: |
| 0 | not yet observed | 100.00 | 100.00 | 0% |
| 1 | +20% | 120.00 | 120.00 | 0% |
| 2 | -25% | 90.00 | 120.00 | 25% |
| 3 | +50% | 135.00 | 135.00 | 0% |
| 4 | -20% | 108.00 | 135.00 | 20% |
| 5 | +10% | 118.80 | 135.00 | 12% |

The *cumulative return* is `(118.80 / 100) - 1 = 0.188 = +18.8%`.
The *maximum drawdown* is `25%`: the decline from **120** at period 1 to
**90** at period 2. The arithmetic average monthly return is `7%`, but adding
the five monthly returns (35%) would give a **wrong** compounded return.

A drawdown is measured against a **previous** peak, never a future one. The
series includes the original wealth as period 0, so a loss in the first month
has a meaningful starting peak.

## Run (C++17, no external dependencies)

From this lesson's directory:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o drawdown_demo
./drawdown_demo
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o drawdown_tests
./drawdown_tests
```

Alternatively, use CMake 3.16 or newer:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/drawdown_demo
```

Expected output for the example (formatted to two decimals):

```text
Period | Return (%) | Wealth | Peak | Drawdown (%)
0 | 0.00 | 100.00 | 100.00 | 0.00
1 | 20.00 | 120.00 | 120.00 | 0.00
2 | -25.00 | 90.00 | 120.00 | 25.00
3 | 50.00 | 135.00 | 135.00 | 0.00
4 | -20.00 | 108.00 | 135.00 | 20.00
5 | 10.00 | 118.80 | 135.00 | 12.00
Final wealth: 118.80
Cumulative return: 18.80%
Maximum drawdown: 25.00%
Worst decline: period 1 to period 2
```

`drawdown_tests` prints `PASS: 50 checks` when all checks pass. An unsuccessful
assertion causes a nonzero program exit code, so a build runner can detect it.

## File map

- `drawdown.hpp`: `PeriodResult`, `DrawdownSummary`, and the input-checked
  `analyze_drawdowns` function.
- `main.cpp`: a standalone demonstration with synthetic returns.
- `tests.cpp`: independent expected wealth, peaks, drawdowns, invalid input,
  and boundary tests.
- `BEGINNERS_GUIDE.md`: explanations, calculations, code walkthrough,
  common mistakes, and exercises with answers.
- `CMakeLists.txt`: reproducible build and test registration.

## Assumptions, limits, and safety

All returns are decimal fractions. A return of `-1.0` is a complete loss;
values below `-1.0` are invalid for a conventional unleveraged wealth index.
Inputs must be finite; initial wealth must be positive and returns nonempty.
Values too large to represent using `double` produce an exception. The
function keeps the *first* worst decline in a tie; if no drawdown occurs, the
reported peak/trough periods remain zero. Wealth is fully reinvested at each
step; there are no deposits or withdrawals. These data are invented.

Historical drawdown depends on sampling frequency and the chosen observation
window. Monthly observations can miss deeper within-month declines. The
calculation ignores fees, tax, inflation, cash contributions, liquidity, and
future uncertainty. It is a descriptive teaching exercise, **not a forecast,
trading-performance claim, or investment advice**.

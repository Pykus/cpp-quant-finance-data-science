# Lesson 08: Historical Value at Risk and Expected Shortfall in C++20

**Question:** Based on the past returns of a fictional portfolio, how bad was a loss at a chosen confidence level, and how bad were the losses beyond that threshold?

This lesson turns daily simple returns into two *historical*, non-parametric tail-loss summaries:

- **Historical Value at Risk (VaR)** uses an empirical loss quantile.
- **Expected Shortfall (ES or CVaR)** averages the worst fraction of losses.

Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) if terms like *quantile*, *confidence*, or *percentile* are unfamiliar.

## Input and conventions

- Input: a sequence of equally weighted, one-period **simple returns** written as decimals. A return of `-0.04` means **-4%**.
- Loss equals **minus return**. A 4% loss is therefore represented by **+0.04** on the loss scale.
- Confidence `c` must satisfy `0 < c < 1`.
- A historical VaR estimate is the loss with **nearest-rank** empirical quantile: sorted loss at 1-based position `ceil(c * n)`.
- ES takes the worst fraction `(1-c)` of observations, using a *fractional observation weight* when `(1-c)*n` is not an integer.
- VaR and ES are **signed**. If every observation is profitable, both can be negative. The code does **not** silently clamp those results to zero.
- All observations have equal weight. No volatility forecasting, market-impact model, or return independence assumption is hard-coded.
- Scaling to currency assumes a fixed positive portfolio value for a single period and no intraperiod flows.

## Worked synthetic example

Ten daily returns (percent):
`-10, -6, -4, -2, 0, +1, +2, +3, +4, +5`.

Losses sorted from smallest to largest (percent):
`-5, -4, -3, -2, -1, 0, +2, +4, +6, +10`.

At **80% confidence**, the nearest rank is `ceil(0.80 * 10) = 8`.
So **VaR = 4%**. The worst `(1-0.80) * 10 = 2` losses are 6% and 10%.
Therefore **ES = (6% + 10%) / 2 = 8%**.

For a synthetic portfolio worth 10,000 currency units, these are **400** and **800** units respectively. They are computed *historical sample summaries*, not promised maximum future losses.

At **85% confidence**, the tail contains `1.5` observations. ES uses the worst loss (10%) in full and the next loss (6%) with a half-weight:
`ES = (10% + 0.5 * 6%) / 1.5 = 8.6666...%`.

## Compile and run

Prerequisite: a C++20 compiler (GCC 11+, Clang 14+, or recent MSVC).

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o tail_risk_demo
./tail_risk_demo

g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o tail_risk_tests
./tail_risk_tests
~~~

Or with CMake:

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
~~~

Expected example output:

~~~text
Observations: 10
Confidence: 80.00%
Historical VaR: 4.00%
Historical expected shortfall: 8.00%
VaR at 10000 currency units: 400.00
ES at 10000 currency units: 800.00
~~~

## Source files

- [risk.hpp](risk.hpp) - reusable, validated, non-mutating risk calculations.
- [main.cpp](main.cpp) - a small demonstration with synthetic returns.
- [tests.cpp](tests.cpp) - deterministic boundary, ordering, example, and invalid-input tests.
- [CMakeLists.txt](CMakeLists.txt) - independent CMake build with CTest registration.
- [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) - definitions, manual arithmetic, C++ walkthrough, questions, and exercises.

## Design decisions and limitations

The returned value is deterministic and permutation-invariant, because it is derived from the **distribution** of observed losses, not their time order. This is a deliberate contrast with drawdown (Lesson 07), which **does** depend on order.

Nearest-rank VaR and fractional-weight ES are explicitly stated choices; other books and libraries use different empirical quantile and ES conventions. Results may differ for small samples, even with identical returns.

This educational example excludes transaction costs, liquidity, correlations in the tails, changing portfolio composition, and regime shifts. In particular, 10 observations are far too few for a credible estimate of a real 99% tail risk. Historical frequency is not a guarantee about future returns.

## Exercises

1. Change confidence from 80% to 90%. Predict VaR and ES before running the code.
2. Reverse the return order. Why do VaR and ES remain unchanged while drawdown could change?
3. Construct four positive-return observations. Explain why signed VaR may be negative.
4. Make a 10-return sample whose two worst losses both equal 5%. What is ES at 80%?
5. Calculate the 85% ES manually, including the fractional tail observation.

Detailed answers and a learning path are in [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md).
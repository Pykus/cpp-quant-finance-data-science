# Lesson 09: Forecast next-day portfolio volatility with EWMA

**Real question:** A portfolio risk analyst needs a quick risk estimate at the close of every trading day. Yesterday's and today's moves should matter more than old returns. How large might tomorrow's daily swing be?

The exponentially weighted moving average (EWMA) is a small, transparent volatility model. Given daily decimal returns `r_t` and decay factor `lambda`, it updates:

`variance_t = lambda * variance_(t-1) + (1-lambda) * r_t^2`

The first variance is seeded with `r_1^2`. Each estimate uses data **only through the current day**, so it can be interpreted as a **next-day forecast**. Volatility is `sqrt(variance)`. We report annualized volatility using `sqrt(252)`, and an illustrative 95% normal-model one-day VaR for a portfolio of a specified currency value. There is no assumed mean return.

## Run

Requires a C++20 compiler; no external packages.

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o ewma_demo
./ewma_demo
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic tests.cpp -o ewma_tests
./ewma_tests
```

Example input: five fictional daily returns `+1%, -2%, +3%, -1%, 0%`, decay `0.94`, portfolio value `100000`. On day 1 the volatility is **1%**, and the 95% normal approximation gives a next-day dollar-loss threshold of about **1644.85**. Day 2 variance rises from **0.000100** to **0.000118**.

## Practical use

Compare risk across successive market closes; for example, alert when estimated daily volatility passes a review threshold. This is a **monitoring component**, not an order execution system.

## Limitations

- Squared returns are measured around a **zero conditional mean**, not a sample mean.
- The first observation determines the seed; early estimates are sensitive to it.
- A normal VaR percentile is not a guaranteed maximum loss; heavy tails and volatility jumps can invalidate the approximation.
- The `sqrt(252)` scaling assumes equal daily time steps and ignores autocorrelation.
- Forecasts require out-of-sample backtesting before business use; this tutorial does not claim calibrated forecast accuracy.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for a complete arithmetic walkthrough and exercises.

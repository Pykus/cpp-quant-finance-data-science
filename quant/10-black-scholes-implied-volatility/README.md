# Lesson 10 — Black–Scholes Implied Volatility in C++20

**Question:** An option trades for a quoted price. What annual volatility would make the Black–Scholes model reproduce that price?

In [Lesson 00](../00-option-basics/), we calculated an option's payoff. In [Lesson 01](../01-monte-carlo-option-pricing/), we simulated option prices. This lesson **inverts a pricing model** using a robust numerical algorithm.

## What you will build

A dependency-free C++20 implementation that:

- Prices **European calls and puts** with continuous interest rates and continuous dividend yields.
- Computes theoretical no-arbitrage lower and upper price bounds.
- Recovers a **Black–Scholes implied volatility** by bracketing and bisection.
- Calculates analytic **vega** as a sensitivity check.
- Validates non-finite inputs, invalid maturities, impossible quotes, and non-convergence.
- Provides deterministic unit tests with known textbook values, parity checks, finite differences, and round trips.

All data are illustrative and synthetic; no live exchange connections or external services are used.

## Core intuition

The Black–Scholes formula produces a price if you supply a volatility, for example

```text
volatility = 0.20 (20% per year)
model call price = 10.45058357
```

An implied-volatility calculator works backwards:

```text
observed call price = 10.45058357
find volatility such that model_price(volatility) = observed price
result = 0.20
```

The model is monotone in volatility for ordinary European calls and puts (positive vega). We can therefore keep an interval with one endpoint below the market price and the other above it, and halve the interval repeatedly.

## Model and notation

Given the spot price `S`, strike `K`, risk-free continuously compounded rate `r`, continuous dividend yield `q`, years to expiry `T > 0`, and volatility `sigma >= 0`:

```text
d1 = (ln(S/K) + (r - q + sigma^2/2)*T) / (sigma*sqrt(T))
d2 = d1 - sigma*sqrt(T)
call = S*exp(-q*T)*N(d1) - K*exp(-r*T)*N(d2)
put  = K*exp(-r*T)*N(-d2) - S*exp(-q*T)*N(-d1)
vega = S*exp(-q*T)*sqrt(T)*phi(d1)
```

Here `N(x)` is the standard normal cumulative distribution function and `phi(x)` is the standard normal density. Implementation uses `std::erfc`, which avoids introducing external math libraries.

**No-arbitrage price bounds:**

```text
call: max(0, S*exp(-q*T) - K*exp(-r*T)) <= price < S*exp(-q*T)
put:  max(0, K*exp(-r*T) - S*exp(-q*T)) <= price < K*exp(-r*T)
```

At exactly the lower bound the solver returns the zero-volatility limit; at the upper bound a finite implied volatility is not identifiable, so the input is rejected. The solver accepts an **absolute price tolerance** (same currency as the option quote), not a percentage.

## Build and run

Requires a C++20 compiler. CMake 3.16 or later is optional.

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o demo
./demo

g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o tests
./tests
```

Or with CMake and CTest:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/implied_volatility_demo
```

Expected demo output (rounded):

```text
Observed call price: 10.45058357
Implied annual volatility: 0.20000000
Repriced option: 10.45058357
Bisection iterations: 36
Call vega (per unit volatility): 37.52403469
```

The number of iterations can vary with tolerance and floating-point behavior.

## API

```cpp
const quant::OptionInputs inputs{100.0, 100.0, 0.05, 0.0, 1.0};
const auto quote = quant::black_scholes_price(
    quant::OptionKind::call, inputs, 0.2);
const auto result = quant::implied_volatility(
    quant::OptionKind::call, inputs, quote);

result.volatility;                // e.g. 0.2, meaning 20% annually
result.iterations;                // bisection steps used
result.absolute_repricing_error;  // model price minus quote, absolute value
```

`quant::arbitrage_bounds` returns the feasible price interval. `quant::black_scholes_vega` returns currency-price change **per unit change in volatility** (a 1% volatility-point move is `0.01 * vega`, approximately).

## Safety and limitations

- A price outside the theoretical interval is not invertible in this model.
- The solver expands the high-volatility bracket to at most `64.0` (6400% annualized); it then reports a bracketing error if the quote cannot be reached.
- Prices very close to the lower or upper limit have low vega: tiny price errors can produce large implied-volatility changes. This is a numerical-identifiability limitation, not a reason to silently fabricate precision.
- The model is for **European exercise** and idealized, continuously paid dividends. It is not a correct American exercise pricer.
- The inputs use **years** and **decimals**: six months is `0.5`, twenty percent volatility is `0.20`, not `20`.
- Floating-point values cannot represent all money prices exactly. This exercise does not model bid/ask spreads, jumps, stochastic volatility, discrete dividends, transaction costs, or market microstructure.
- `std::invalid_argument` communicates invalid inputs; `std::runtime_error` communicates failure to converge.

## Reading order

Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md), then read `implied_volatility.hpp`, run `main.cpp`, and inspect `tests.cpp`. Try the exercises before reading the proposed solutions.

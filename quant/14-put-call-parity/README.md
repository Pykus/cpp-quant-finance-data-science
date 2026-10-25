# Lesson 14 — European put–call parity

**Goal:** understand a no-arbitrage relationship between matching call and put
options, and implement an auditable numerical checker in C++20.

## Problem

A European call and a European put share a strike and expiration date.
Assuming no dividends, a liquid frictionless market, and a continuously
compounded risk-free interest rate, their prices satisfy:

    Call - Put = Stock - Strike * exp(-Rate * Years)

Create a function that calculates both sides, reports the absolute difference,
and decides whether it fits a caller-specified tolerance.

**Example:** stock 100, strike 100, rate 5%, one year, call 10.
The discounted strike is about 95.12294, so the matching put is approximately
5.12294. A pair quoted as call 10, put 5.13294 differs by about 0.01.

## C++20 implementation

- `EuropeanOptionPair` collects the inputs without global state.
- `ParityCheck` returns the quoted difference, theoretical difference,
  absolute error and an explicit Boolean.
- `check_parity` checks finite, positive stock/strike, nonnegative option prices
  and expiry, and a nonnegative tolerance.
- `std::exp` discounts the strike; `std::abs` measures deviation.
- Invalid inputs throw `std::invalid_argument`; nonfinite calculations throw
  `std::overflow_error` rather than quietly returning a misleading answer.

This checker **does not prove an executable arbitrage**. Bid/ask spreads,
borrowing costs, dividends, trading fees, and stale quotes can explain apparent
discrepancies. It uses an *absolute* price tolerance in the same currency
units as the option prices.

Time: **O(1)** per check. Auxiliary space: **O(1)**.

## Build and test

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o parity_tests
./parity_tests
```

Tests cover the worked example, inconsistent quotes, invalid inputs,
overflow and 10,000 deterministic synthetic parity-consistent pairs.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) before changing the formulas.
**Exercise:** extend the equation to a known continuous dividend yield.

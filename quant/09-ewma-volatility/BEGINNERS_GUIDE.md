# Beginner's guide: why old and new returns get different weights

## 1. Who uses a volatility estimate?

Imagine a risk analyst watching a **fictional $100,000 portfolio**. A return of `-0.02` means the portfolio fell 2% that day. The analyst wants a forecast of how variable returns may be **tomorrow**.

A simple historical standard deviation gives old and new observations similar weight inside its window. EWMA updates a running estimate and **gradually forgets older returns**.

## 2. Understand variance

A return of 1% is stored as `0.01`, not `1`. Squaring gives `0.01 * 0.01 = 0.0001`. Squared returns are always nonnegative, whether the market rises or falls.

We initialize the running variance with the first squared return. This is a simple and explicit model choice, **not** an unbiased statistical estimate.

## 3. Work through day 2

We have day 1: `r1 = +0.01`, so `v1 = 0.0001`.

Day 2: `r2 = -0.02` with `lambda = 0.94`.

```
v2 = 0.94 * 0.0001 + 0.06 * (-0.02)^2
   = 0.000094 + 0.000024
   = 0.000118
daily volatility = sqrt(0.000118) = about 0.010863
```

That is approximately **1.0863% daily volatility**. The sign of yesterday's return disappears when we square it; EWMA captures **magnitude of moves**, not direction.

## 4. Why does lambda matter?

The latest squared return gets weight `1 - lambda`. With `lambda = 0.94`, that is 6%. Older information persists but decays. A smaller lambda reacts faster to new large moves; a lambda very close to 1 changes slowly.

For `lambda = 0`, the model uses only the latest squared return. Lambda must be between 0 inclusive and 1 exclusive in this implementation.

## 5. Translate percentage risk into an illustrative dollar figure

A standard-normal 95th percentile is approximately `z = 1.64485`. With **zero mean** and a daily volatility of 1%, an illustrative one-day 95% VaR is:

`1.64485 * 0.01 * $100,000 = $1,644.85`.

In this simplified model, a loss **larger** than this would occur on roughly 5% of days *only if model assumptions are correct*. VaR is not a maximum loss. Financial crises can create much bigger losses.

## 6. Read the C++ data structures

- `std::span<const double>` accepts a view of returns without copying and prohibits modifying them.
- `std::vector<RiskSnapshot>` returns one forecast per observed day.
- `std::isfinite` rejects missing, infinite or invalid numerical values.
- The model throws a clear error if arithmetic would overflow.

## 7. Exercises

1. Calculate day 3 variance by hand with `r3 = +0.03`. Confirm the result is `0.00016492`.
2. Explain why a **-3%** move and a **+3%** move have the same effect on next-day EWMA variance.
3. Change lambda from `0.94` to `0.50`. Which reacts more to a 3% shock?
4. Run the example with portfolio value `$200,000`. Does estimated volatility change? Does dollar VaR change?
5. Investigate what happens if the earliest return is zero. Why might initializing with a longer historical variance estimate be preferable in practice?

## 8. Next step

A serious risk engine would compare the forecast at day `t` with realized movement on day `t+1`, measure prediction errors on data never used to fit parameters, and use more robust tail-risk methods.

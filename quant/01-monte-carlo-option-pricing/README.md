# Quant Lesson 01: From Option Payoffs to Monte Carlo (C++17)

**Before this lesson:** Complete [Lesson 00: Your First Option Calculation](../00-option-basics/). It explains what a call option and its payoff are, with simple numbers and C++ code.

This lesson answers a new question: **What is an option worth today if we do not know the future share price?**

## Step 1. One future price is not enough

In Lesson 00, we calculated the payoff when a share ends at 120 and the strike is 100: **20**.

But the future price is unknown. It might be 90, 100, 120, or something else. A computer can explore many possible outcomes instead of guessing just one.

## Step 2. What is Monte Carlo?

**Monte Carlo simulation** means using random numbers to explore many possible outcomes.

Imagine a very simple classroom exercise:

| Possible future price | Call payoff with strike 100 |
| --- | --- |
| 90 | 0 |
| 110 | 10 |
| 120 | 20 |
| 100 | 0 |

The average payoff in this *illustrative table* is `(0 + 10 + 20 + 0) / 4 = 7.5`.

This is only an example of averaging; these four prices were chosen for teaching and **are not a financial pricing model**. In a real simulation, we need a justified way to generate the prices.

## Step 3. The model's five inputs

Our program uses:

| Name | Example | Meaning |
| --- | --- | --- |
| `spot` | 100 | Share price today |
| `strike` | 100 | Agreed purchase price |
| `rate` | 0.05 | Annual continuously compounded interest rate (5%) |
| `volatility` | 0.20 | Assumed annual price variability (20%) |
| `maturity` | 1.0 | Years until the option expires |

**Volatility** describes how much prices may move. It is not a prediction of whether they will go up or down.

## Step 4. Generate possible future prices

The program uses a standard mathematical model called **geometric Brownian motion**. Under its assumptions, a simulated future price is:

`future = spot * exp((rate - volatility²/2)*maturity + volatility*sqrt(maturity)*Z)`

Here:

- `exp(x)` means the exponential function; it keeps modeled prices positive.
- `sqrt(x)` means the square root.
- `Z` is a random number from the **standard normal distribution**: most values are near zero, with larger positive and negative values less common.
- `volatility²` means volatility multiplied by itself.

You do **not** need to memorize this formula. For now, understand its job: **turn one random number into one possible future share price**. The model makes strong simplifying assumptions, including constant volatility and interest rate.

## Step 5. Calculate and average payoffs

For each simulated future price, the program applies the rule from Lesson 00:

`payoff = max(future - strike, 0)`

It then averages the payoffs. Because the money would be received in the future, the model converts that average into today's value using a **discount factor**:

`discount = exp(-rate * maturity)`

So the simplified idea is:

`option price today = average simulated payoff * discount`

This is **risk-neutral pricing under the model**, not a claim that real share prices grow at the interest rate.

## Step 6. Why are there pairs of simulations?

The implementation uses a technique called **antithetic variates**. For every random value `Z`, it also tries `-Z`. It averages those two payoffs as **one pair**.

Why? Positive and negative shocks can partly balance each other. This often makes the final estimate less noisy without changing the underlying pricing model.

## Step 7. How certain is the answer?

Running a finite number of random scenarios gives an **estimate**, not an exact answer.

- **Standard error:** approximately how much the estimated average would vary across repeated simulations.
- **95% confidence interval:** an approximate range constructed from the estimate and its standard error. Here it is `estimate ± 1.96 * standard_error`. This describes simulation noise *under the assumed model*, not uncertainty about real markets.

The program also prints a **Black–Scholes analytical price**. This is a formula for the same model, used here as a reference to check whether our simulation is behaving sensibly.

The detailed mathematical expressions are implemented in [pricing.hpp](pricing.hpp). Read the numbered steps above first; the formulas and code can be studied afterwards.

## Step 8. Build and run

From this lesson's directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/quant_call
```

Example output format (exact simulated values may vary across C++ standard-library implementations):

```text
black_scholes=10.450584
monte_carlo=...
standard_error=...
ci95_lower=...
ci95_upper=...
paths=200000
```

`paths=200000` means the program simulated **100,000 pairs**, each containing two possible future prices.

## Step 9. Exercises, from easy to harder

1. **Easy:** Explain in your own words why an option with strike 100 has zero payoff when the share ends at 90.
2. **Easy:** Identify the five model inputs in [main.cpp](main.cpp).
3. **Medium:** Run the program twice. Why are the results the same on your machine? (Hint: the random seed is fixed at 42.)
4. **Medium:** Change the number of pairs from 100,000 to 10,000. What happens to the standard error?
5. **Advanced, optional:** Study the analytical Black–Scholes calculation in `pricing.hpp` and compare it with the Monte Carlo result.

## Important limitations

The model assumes constant volatility and interest rate, lognormal prices, no dividends, and European exercise only. The 95% interval uses a large-sample normal approximation. Exact random sequences may differ across C++ implementations. Extreme inputs can cause numerical overflow. These are educational model calculations, not market forecasts or investment advice.

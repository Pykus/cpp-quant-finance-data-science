# Beginner's Guide — From an Option Price to Implied Volatility

You can work through this lesson without economics, statistics, or C++ experience. We explain the financial vocabulary, the mathematical recipe, and why the C++ implementation works. The example numbers are invented and deliberately simple.

## 1. The market question

Imagine you have a ticket that lets you **buy one share for 100** in one year, even if the share then costs 130. This is a **call option**.

If you exercise when the share costs 130, your payoff is:

```text
max(130 - 100, 0) = 30
```

If the share instead costs 90, you would not pay 100 for it:

```text
max(90 - 100, 0) = 0
```

The payoff is what the ticket is worth **at expiry**. It is not the price you pay to buy the ticket **today**.

This difference matters because nobody knows next year's share price.

A **put option** reverses the right: it lets you sell a share at a predetermined strike. Its payoff at expiry is `max(strike - future_price, 0)`.

A **European** option can be exercised only at expiration. An **American** option may permit exercise earlier. The Black–Scholes formula in this lesson models European options.

## 2. Five financial inputs, in everyday language

| Variable | Name | Interpretation | Example |
| --- | --- | --- | --- |
| `S` | Spot | Share price now | 100 |
| `K` | Strike | Agreed purchase or sale price | 100 |
| `T` | Time | Years remaining until expiry | 1.0 |
| `r` | Risk-free rate | Continuously compounded annual rate | 0.05 = 5% |
| `q` | Dividend yield | Continuously compounded annual dividend yield | 0.00 |
| `sigma` | Volatility | Model parameter describing the scale of annual share-price uncertainty | 0.20 = 20% |

Six months means `T=0.5`, not `T=6`. Two and a half percent means `0.025`, not `2.5`.

The spot price is **not** the strike. They happen to match in our first example, but need not match in general.

### Mini exercise

If the future share price is 75 and the strike is 90:

- Call payoff = `max(75-90,0) = 0`.
- Put payoff = `max(90-75,0) = 15`.

Try the same calculation with a future price of 105.

## 3. What is volatility?

Volatility describes **how widely a model allows returns to vary**. In the Black–Scholes model, annual volatility of `0.20` means a 20% annualized standard deviation of logarithmic returns.

It does **not** mean that the share must gain or lose exactly 20% next year. It is not a probability that the option finishes in the money. A high volatility increases the range of plausible outcomes.

Why does the call owner care? The owner benefits from a very high price, but can choose not to exercise after a very low price. This one-sided payoff generally becomes more valuable as the range of potential share prices increases.

The same basic argument applies to put options.

## 4. Interest and dividends: money at different dates

Receiving 100 next year is different from receiving 100 now.

At a continuously compounded 5% annual rate, the present value of 100 paid in one year is

```text
present value = 100 * exp(-0.05 * 1)
              = 95.12294245
```

Here `exp(x)` is the mathematical exponential function, and C++ supplies it as `std::exp(x)`.

A stock that continuously distributes dividends also behaves differently from a non-dividend-paying stock in this model. We use:

```text
discounted spot   = S * exp(-q*T)
discounted strike = K * exp(-r*T)
```

For the example `S=100, K=100, q=0, r=5%, T=1`:

```text
discounted spot   = 100.00000000
discounted strike =  95.12294245
```

That difference, approximately 4.87705755, will reappear in our price bounds.

## 5. A mathematical tool: the normal distribution

The **standard normal distribution** is the familiar bell-shaped curve centered at zero. Its density is written `phi(x)`. Its cumulative distribution function (CDF), written `N(x)`, measures the probability of a standard normal value being less than or equal to `x`.

Useful landmarks:

```text
N(0)       = 0.5
N(1)       approximately 0.8413
N(-1)      approximately 0.1587
N(large positive number) approaches 1
N(large negative number) approaches 0
```

Our program evaluates the CDF using the complementary error function:

```cpp
double normal_cdf(double x) {
    return 0.5 * std::erfc(-x / std::numbers::sqrt2);
}
```

You do not have to memorize why this identity holds to understand bisection. The key point is that `normal_cdf` converts a standardized score into a cumulative probability.

## 6. The Black–Scholes calculation

For strictly positive `sigma`, define two intermediate numbers:

```text
d1 = [ln(S/K) + (r-q + sigma*sigma/2)*T] / (sigma*sqrt(T))
d2 = d1 - sigma*sqrt(T)
```

Then the European call price is:

```text
C = S*exp(-q*T)*N(d1) - K*exp(-r*T)*N(d2)
```

And the European put price is:

```text
P = K*exp(-r*T)*N(-d2) - S*exp(-q*T)*N(-d1)
```

Do not confuse a **call option price**, `C`, with its later **payoff**.

### Worked numbers

Use:

```text
S = 100
K = 100
r = 0.05
q = 0
T = 1
sigma = 0.20
```

First compute `ln(100/100)=0`, `sqrt(1)=1`, and `sigma^2/2=0.02`.

So:

```text
d1 = (0 + (0.05 + 0.02)*1) / (0.20*1) = 0.35
d2 = 0.35 - 0.20 = 0.15
```

From the normal CDF, `N(0.35)` is approximately 0.63683065 and `N(0.15)` approximately 0.55961769.

```text
C = 100*N(0.35) - 100*exp(-0.05)*N(0.15)
  = approximately 10.45058357
```

The corresponding put is approximately `5.57352602`.

These reference numbers are also checked in `tests.cpp`, so changing the program carelessly can be detected.

## 7. Turning the calculation around

The pricing function accepts a volatility and returns a price:

```text
price = BlackScholes(sigma)
```

But a market shows you a **price**. You can ask the reverse question:

```text
Which sigma makes BlackScholes(sigma) equal the observed price?
```

The answer is called **implied volatility** (IV).

For example, if we observe a call quote of 10.45058357 for the inputs above, the model's implied volatility is approximately 0.20 (20% annually).

Important distinction: **historical volatility** is estimated from past return data; **implied volatility** is obtained by inverting today's option price in a specified mathematical model. They need not agree.

## 8. Why use bisection?

You have probably solved a puzzle where you guess a number, are told "too high" or "too low," and keep narrowing the possibilities.

Bisection does exactly that.

For ordinary positive-volatility European calls and puts, the Black–Scholes option price grows as volatility grows. Consequently, if one volatility produces a price below the market quote and another produces a price above it, the answer lies between them.

We begin with:

```text
low volatility  = 0
high volatility = 0.25
```

If the high endpoint is too cheap, double it:

```text
0.25 -> 0.50 -> 1.00 -> 2.00 ...
```

Once the model quote at `high` is at least the observed quote, the interval contains the solution. Each iteration:

1. Compute `midpoint = (low + high)/2`.
2. Price the option at `midpoint`.
3. If its price is too low, replace `low` with `midpoint`.
4. Otherwise replace `high` with `midpoint`.
5. Stop when the absolute price difference is within the requested tolerance.

For our benchmark the answer converges near `sigma=0.20`.

### Why not just try random values?

A random guess could jump around indefinitely. Bisection guarantees a systematically shrinking interval after a valid bracket has been established. It is slower than some derivative-based methods but particularly easy to reason about, test, and maintain.

The implementation uses `std::midpoint` (C++20), which avoids some overflow problems of `(low+high)/2`.

## 9. Can every quoted price have an IV?

No. Simple arbitrage arguments constrain reasonable prices **inside this model**.

For a European call:

```text
lower = max(0, discounted_spot - discounted_strike)
upper = discounted_spot
```

For a European put:

```text
lower = max(0, discounted_strike - discounted_spot)
upper = discounted_strike
```

Take our example:

```text
call lower = max(0, 100 - 95.12294245) = 4.87705755
call upper = 100
```

A call quote of 3 would contradict those assumptions: the calculator rejects it instead of printing an invented volatility.

At the **lower bound**, the zero-volatility limit has that price. The solver returns `sigma=0` and needs no iterations.

At the **upper bound**, a finite volatility cannot normally reproduce the exact limit. The solver rejects the quote rather than pretend that a finite solution exists.

Quotes extremely close to either endpoint can be difficult to invert accurately even when mathematically admissible.

## 10. Vega: how sensitive is the price?

Vega tells us how much the model option price changes when volatility changes a little.

The Black–Scholes vega is:

```text
vega = S * exp(-q*T) * sqrt(T) * phi(d1)
```

Our benchmark has:

```text
vega approximately 37.52403469
```

Vega is measured per **1.0 absolute volatility unit**. For a change from `0.20` to `0.21` (one volatility percentage point), a first-order price estimate is:

```text
price change approximately vega * 0.01
             approximately 0.37524035
```

That is an approximation, not an exact repricing.

When vega is very small, option prices barely respond to volatility. Inverting the option price then becomes numerically unstable: tiny quote changes can imply very different volatility estimates.

This is why `ImpliedVolatilityResult` reports the **repricing error**. The reported digits of volatility are not a guarantee of economic precision.

## 11. Put-call parity: a free consistency test

For European options with the same strike and maturity:

```text
call - put = discounted_spot - discounted_strike
```

For our example:

```text
10.45058357 - 5.57352602 = 4.87705755
100 - 95.12294245         = 4.87705755
```

Both sides match within floating-point rounding.

This equality is useful because it links **two independently computed prices**. Tests that only check one output can accidentally preserve an incorrect formula; parity checks catch additional errors.

## 12. Reading the C++ source file

Open `implied_volatility.hpp` and follow the functions from top to bottom.

- `OptionKind` is an `enum class`. It represents call versus put in a type-safe way.
- `OptionInputs` collects five required numbers into one object.
- `PriceBounds` groups two related values.
- `[[nodiscard]]` encourages callers to use returned calculations.
- `detail::discounted_values` checks inputs and computes present values.
- `black_scholes_price` computes a call or put price.
- `black_scholes_vega` computes an analytic sensitivity.
- `implied_volatility` calls the pricing function repeatedly until it matches a quote.

### Why reject NaN and infinity?

A double can store special values such as `NaN` ("not a number") and `infinity`. Comparisons involving NaN can behave unexpectedly: `NaN < 0` is false, and `NaN >= 0` is also false.

A risk calculator that silently accepts such values may produce misleading output. We use `std::isfinite` and throw `std::invalid_argument` for invalid numeric inputs.

### Why exceptions?

The program distinguishes **a valid result** from **an error**. If a price violates the model's theoretical bounds, returning a made-up `0.2` would be dangerous. Exceptions allow the caller to handle the problem explicitly.

`std::runtime_error` indicates a different problem: the requested numerical precision was not obtained within the allowed iterations.

## 13. Building it yourself

With a C++20 compiler:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o demo
./demo
```

Then build the tests:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o tests
./tests
```

If CMake is installed:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

### What tests look for

The tests use several strategies, rather than merely printing "success":

1. Known published textbook call and put reference values.
2. Put-call parity, including nonzero dividends.
3. Round trips: `price -> implied volatility -> same price`.
4. Price monotonicity as volatility increases.
5. Analytic vega versus a finite-difference approximation.
6. Invalid inputs and impossible market quotes.
7. Iteration and precision-contract failures.

All values are deterministic and synthetic.

## 14. Common beginner mistakes

**Mistake: Passing 20 for 20% volatility.**
Correct: `0.20`. Passing 20 means 2000% annual volatility.

**Mistake: Passing twelve for twelve months.**
Correct: `T=1.0` year.

**Mistake: Treating the payoff as today's price.**
The payoff describes a possible outcome at expiry; today's fair-value estimate also accounts for time and uncertainty.

**Mistake: Assuming IV forecasts actual future volatility.**
IV is a parameter that matches today's price in a particular model; it is not a guaranteed forecast.

**Mistake: Ignoring the option's exercise type.**
American put options can have early-exercise value that this European formula does not capture.

**Mistake: Assuming a tiny repricing error means a precise IV.**
Low vega means even a good price fit may leave IV poorly identified.

**Mistake: Using a single quote with no bid/ask spread.**
Real options have bid/ask prices and imperfect liquidity. This educational program does not model those effects.

## 15. Your exercises

### Exercise A — Payoffs

For strike `K=110` and expiry price `S_T=125`, calculate the call and put payoff. Repeat for `S_T=95`.

### Exercise B — Rates

Using `K=100, r=0.05, T=1`, compute the present value of the strike. What happens if `r=0`?

### Exercise C — Model inversion

Change the demo's option input from `T=1.0` to `T=0.5`, and generate a fresh quote with `black_scholes_price` using volatility 0.20. Does `implied_volatility` recover 0.20?

### Exercise D — Put and call

In the demo, change `OptionKind::call` to `OptionKind::put`. Why must you also change the observed quote?

### Exercise E — Vega

Use the analytic vega and a small price difference `price(0.201)-price(0.200)`. Compare the difference with `vega(0.200)*0.001`. Why are they only approximately equal?

### Exercise F — Reject a bad price

For the benchmark input, ask the solver to invert a call price of 3. What happens, and why is rejecting it better than returning a number?

### Exercise G — Explore accuracy

Run the program with price tolerance `1e-6` and `1e-12`. Record the iteration counts and volatility estimates. Why does the tighter tolerance usually require more work?

## 16. Exercise answers and checkpoints

**A.** With `S_T=125`, call payoff 15, put payoff 0. With `S_T=95`, call payoff 0, put payoff 15.

**B.** `100*exp(-0.05)=95.12294245`. With `r=0`, the present value is 100.

**C.** Yes, it should recover a value near `0.20` as long as you generate the quote with the new maturity. Keeping the old quote while changing time changes the financial problem.

**D.** The call and put are different contracts. Their model prices differ, even with the same spot, strike, rate, time, and volatility.

**E.** Vega is a local derivative, while the actual price difference accumulates a small amount of curvature as volatility changes.

**F.** A call worth 3 is below the example's theoretical lower bound of approximately 4.8771. The function throws `std::invalid_argument`.

**G.** A smaller acceptable repricing error generally means more interval halvings. Near very low vega, even tiny price differences might not pin down volatility to many digits.

## 17. What to learn next

- **Numerical analysis:** bisection, stopping criteria, floating-point rounding.
- **Options theory:** intrinsic value, time value, put-call parity.
- **Risk management:** compare volatility estimated from historical returns with IV extracted from quotes.
- **C++20:** `std::midpoint`, `std::numbers`, `std::isfinite`, `std::invalid_argument`, and tests.
- **More advanced models:** implied-volatility smile, discrete dividends, binomial trees for American exercise, and stochastic volatility.

### Final self-check

Can you explain in one sentence why a market option price and the formula's volatility are different kinds of inputs?

Can you explain why a "too high/too low" guessing game is a legitimate numerical method?

Can you explain why a report of `volatility=0.20000000` is not necessarily a guarantee of eight digits of *economic* precision?

If you can answer those questions, you understand the core idea of implied-volatility inversion.

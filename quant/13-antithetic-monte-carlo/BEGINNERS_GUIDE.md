# First-year guide: variance reduction in option pricing

This guide starts with basic arithmetic, assumes no prior study of financial derivatives, and explains why the two solutions use **different statistical sample counts** despite evaluating the same number of payoffs.

## 1. What is a call option?

A **European call** is the right, but not the obligation, to buy one share for a fixed **strike price** on one future **expiry date**. If a call has strike 100, its future payoff is:

| Share price at expiry | Payoff |
| ---: | ---: |
| 80 | 0 |
| 100 | 0 |
| 110 | 10 |
| 135 | 35 |

If the share is worth 80 and the right lets you buy it for 100, you simply do not exercise it. Therefore a call cannot have a negative payoff at expiry.

The equation is `payoff = max(future_share_price - strike, 0)`. It describes how much the right will be worth **at expiry**, not what it costs **today**.

## 2. Simulating one possible future

No one knows the future share price. Instead, a mathematical model generates many hypothetical outcomes. Monte Carlo simulation means drawing random scenarios, computing a result for each, and averaging those results.

For this lesson, the simplified risk-neutral model gives the terminal price:

~~~text
S_T = S_0 * exp((r - sigma^2/2) * T + sigma * sqrt(T) * Z)
~~~

- `S_0`: price of the share today, for example 100.
- `S_T`: possible simulated future price.
- `r`: annual continuously compounded risk-free rate, such as 0.05 for 5%.
- `sigma`: annual volatility, such as 0.20 for 20%.
- `T`: time in years until expiry.
- `Z`: a *standard normal* random number with mean 0 and variance 1.
- `exp(x)`: exponential function.

Why subtract `sigma²/2`? Exponentiating a normally distributed random number shifts its expectation. The correction is part of the geometric Brownian motion formula that yields the intended model's risk-neutral expected growth. These probabilities are mathematical pricing probabilities, not a guarantee about the real market.

### A numerical example

Choose `S_0=100`, `K=100`, `r=0`, `sigma=0.20`, `T=1`. Then the deterministic part is `-0.02` and the random part is `0.2*Z`.

With `Z=+1`:

~~~text
S_T = 100 * exp(-0.02 + 0.2)
    = 100 * exp(0.18)
    ≈ 119.72
call payoff ≈ 19.72
~~~

With `Z=-1`:

~~~text
S_T = 100 * exp(-0.02 - 0.2)
    = 100 * exp(-0.22)
    ≈ 80.25
call payoff = 0
~~~

Those are the two mirror-image scenarios that will form one antithetic pair.

## 3. Discounting future money

One unit of money received next year is not worth the same as one unit today if money earns interest. A standard continuously compounded discount factor is `exp(-r*T)`.

At 5% for one year, `exp(-0.05)≈0.95123`. A future payoff of 10 has a modeled present value near 9.51.

For every simulated scenario we calculate the **discounted payoff**:

~~~text
Y = exp(-r*T) * max(S_T - K, 0)
~~~

Under the model's risk-neutral assumptions, the theoretical option value is the *expected* (long-run average) discounted payoff.

## 4. Why an ordinary Monte Carlo average is uncertain

Imagine four independent discounted payoffs: `0, 8, 12, 20`. Their sample mean is `(0+8+12+20)/4 = 10`.

Running a new simulation generally gives a different mean: these fluctuations are **sampling error**, not necessarily programming errors.

For independent observations `x1,...,xn`:

~~~text
sample_mean     = (x1 + ... + xn) / n
sample_variance = sum((xi - sample_mean)^2) / (n - 1)
standard_dev    = sqrt(sample_variance)
standard_error  = standard_dev / sqrt(n)
~~~

The *standard deviation* measures the spread of individual observations. The *standard error* measures the uncertainty in the **average**. Increasing the number of independent observations generally shrinks the standard error approximately as `1/sqrt(n)`.

## 5. The antithetic trick: use Z and -Z

A standard normal distribution is symmetric: if `Z` is standard normal, then `-Z` has the same distribution. Instead of generating only unrelated normal values, generate a value `Z` and then calculate **two** discounted payoffs — one using `Z` and one using `-Z`.

Each scenario separately is still valid. Their average is also a valid estimator of the original expected payoff. But the two scenarios are **dependent**: if you know `Z`, then you know `-Z` exactly.

For the earlier hand calculation, the two payoffs were about 19.72 and 0, yielding **one pair mean** of `(19.72+0)/2 = 9.86`.

~~~cpp
const double z = normal(rng);
const double a = payoff(z);
const double b = payoff(-z);
stats.add(0.5 * a + 0.5 * b);
~~~

Read line by line: draw one normally distributed value, price its payoff, price the mirrored payoff, then record one *pair average*. The final line executes only once per pair.

## 6. The statistical reason it can be more precise

Let `A` be the discounted payoff from `Z` and `B` from `-Z`. The pair average is `M=(A+B)/2`.

A useful variance identity is:

~~~text
Var(A + B) = Var(A) + Var(B) + 2 * Cov(A,B)
~~~

The variance of an average follows:

~~~text
Var(M) = (Var(A) + Var(B) + 2 * Cov(A,B)) / 4
~~~

Both individual payoffs have the same distribution, so write `Var(A)=Var(B)=v`:

~~~text
Var(M) = (v + Cov(A,B)) / 2
~~~

For a call, a positive shock tends to increase one payoff while the mirrored negative shock tends to decrease the other. This usually gives a **negative covariance**, which reduces variance. The method does **not** guarantee improvement for every payoff function or model.

## 7. Compare equal computational budgets

To avoid cheating, compare the estimators at the same number of **payoff evaluations**.

| Method | Normal draws | Payoff evaluations | Independent statistical observations |
| --- | ---: | ---: | ---: |
| Ordinary | 2N | 2N | 2N separate payoffs |
| Antithetic | N | 2N | N pair averages |

This distinction matters for the standard-error formula: using 2N as the antithetic sample count would falsely treat dependent payoffs as independent and generally understate uncertainty.

The code uses `stats.add(...)` once per independent ordinary payoff and once per *independent pair mean* in the antithetic method.

## 8. What does a 95% confidence interval mean?

The program reports an approximately normal 95% confidence interval:

~~~text
[estimated_price - 1.95996398454*SE,
 estimated_price + 1.95996398454*SE]
~~~

Example: if the mean is 10.45 and `SE=0.03`, the margin is about `0.0588` and the interval is approximately `[10.3912, 10.5088]`.

An individual interval can miss the theoretical price. The 95% statement concerns the long-run coverage rate of the **interval-producing procedure**, assuming the model and normal approximation apply. The interval does not cover errors from an incorrect volatility, interest rate, or pricing model.

When samples are tiny or payoffs are very skewed, the normal approximation can be unreliable. That is why the lesson uses substantial sample counts and calls these intervals *approximate*.

## 9. Welford's algorithm: statistics without storing all paths

The implementation avoids an enormous vector of simulated prices. It updates the mean and sample variance online:

~~~cpp
++count;
const double delta = x - mean;
mean += delta / static_cast<double>(count);
squared += delta * (x - mean);
~~~

- Increase the number of independent statistical observations.
- Measure the distance from the old mean.
- Adjust the running mean by that distance divided by the new count.
- Update the sum of squared deviations using the old and new means.

At the end, `sample_variance = squared/(count-1)` and `standard_error = sqrt(sample_variance/count)`.

This needs only **constant additional memory** and is more numerically stable than `average(x²) - average(x)²` when observations have large means.

The program requires at least two pairs so `count-1` is nonzero for both sampling modes.

## 10. Reading the C++20 types

~~~cpp
struct EuropeanCall {
    double spot, strike, rate, volatility, maturity;
};
enum class Method { independent, antithetic };
~~~

A `struct` groups the five model inputs. A strongly typed enumeration makes the chosen algorithm readable. `double` is appropriate for numerical approximation, not exact currency accounting.

~~~cpp
[[nodiscard]] Estimate estimate(
    const EuropeanCall& option,
    std::size_t pairs,
    std::uint64_t seed,
    Method method);
~~~

- `const EuropeanCall&` avoids a copy and prohibits modifying the caller's object through this reference.
- `std::size_t` is the standard type for counts and container indexing.
- `std::uint64_t` is an explicit-width unsigned seed.
- `[[nodiscard]]` asks the compiler to warn if the result is discarded.
- There is deliberately **no `noexcept`**: invalid inputs and numerical overflow produce exceptions. A throwing function marked `noexcept` would instead terminate the program.
- Automatic local variables own their resources by lifetime (RAII); no manual allocations, deletes or threads are necessary.

## 11. Randomness and platform differences

~~~cpp
std::mt19937_64 rng(seed);
std::normal_distribution<double> normal(0.0, 1.0);
double z = normal(rng);
~~~

`std::mt19937_64` is a reproducible pseudorandom engine for a seed, while `std::normal_distribution` creates normal values from engine output.

A fixed seed reproduces results with the same standard-library implementation. C++ does **not** require every standard library to use an identical algorithm for normal-distribution sampling, so the exact numerical Monte Carlo output may differ across platforms. Tests therefore verify invariant mathematical properties rather than hardcoding one sampled price.

## 12. Check against Black–Scholes

The project provides a closed-form Black–Scholes call price under the same model assumptions. It uses the standard normal cumulative distribution computed through `std::erfc`.

For `spot=100`, `strike=100`, `rate=0.05`, `volatility=0.20`, `maturity=1`, the model's analytic price is approximately **10.450584**.

Our simulated estimates should be close to it, with deviations on the order of their estimated standard errors. This analytic benchmark is not a live market quotation.

## 13. What the demonstration actually proves

With `N=100,000` pairs, each estimator computes **200,000 discounted payoffs**, but the independent estimator has **200,000 observations** and the antithetic estimator has **100,000 pair-mean observations**.

A smaller observed standard error for the latter means it is more precise **for this particular simulation fixture**. It does not establish that every possible payoff benefits from pairing.

If you multiply the independent sample count by four, the standard error usually drops by about a factor of two, not four, assuming a finite variance. Reducing variance through careful pairing can therefore save computation.

## 14. Common mistakes and boundaries

- Averaging future payoffs without discounting them to the present.
- Counting correlated paired payoffs as if each were independent.
- Confusing the standard deviation of payoffs with the standard error of the mean.
- Comparing two methods with different numbers of expensive payoff evaluations.
- Assuming different C++ standard libraries generate identical normal draws from a seed.
- Treating confidence intervals as guaranteed coverage for an individual experiment.
- Mistaking a simplified pricing model for a complete market trading system.

This implementation ignores dividends, early exercise, changing volatility, jumps, bid/ask spreads and transaction costs. It checks inputs and numerical overflow but remains a teaching model.

## 15. Try these questions

1. Why does a call payoff equal zero if the share ends below the strike?
2. Why are `Z` and `-Z` dependent even though each has a standard normal distribution?
3. With 8,000 antithetic pairs, how many payoffs are calculated, and how many independent observations enter the variance estimate?
4. If the standard error is 0.04, what is the approximate 95% interval margin?
5. Why does the implementation not mark the pricing function `noexcept`?
6. Change the payoff to that of a put option. Does negative pairing always give a lower estimated standard error?

Make a prediction, then edit `demo.cpp`, compile, and run the tests before drawing conclusions.

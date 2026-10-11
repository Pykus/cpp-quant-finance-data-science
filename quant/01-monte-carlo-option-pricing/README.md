# Quant Lesson 01: European Call Pricing with Monte Carlo (C++17)

This lesson prices a European call option under the **Black–Scholes geometric Brownian motion model**, compares an analytical solution with a Monte Carlo estimate, and computes an approximate 95% confidence interval.

## Mathematical model

Under the risk-neutral measure, the terminal asset price is

`S(T) = S(0) * exp((r - sigma^2 / 2) * T + sigma * sqrt(T) * Z)`, with `Z ~ N(0, 1)`.

The discounted call payoff is `exp(-r*T) * max(S(T) - K, 0)`.

The analytical Black–Scholes price is `S(0)*Phi(d1) - K*exp(-r*T)*Phi(d2)`, where `d1 = [ln(S(0)/K) + (r + sigma^2/2)*T] / (sigma*sqrt(T))` and `d2 = d1 - sigma*sqrt(T)`.

The simulation uses **antithetic variates**: each normal draw `Z` is paired with `-Z`. Each *pair average* is treated as one independent sample when estimating the standard error. This is essential: treating both correlated payoffs as independent would misstate uncertainty. The 95% interval uses the normal approximation `estimate +/- 1.96 * SE`; it is not a guaranteed coverage interval for small samples.

## Build, run and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/quant_call
```

Sample output from the tested environment (the standard-library normal distribution sequence can differ between C++ implementations):

```text
black_scholes=10.450584
monte_carlo=10.456195
standard_error=0.023220
ci95_lower=10.410684
ci95_upper=10.501706
paths=200000
```

The sample parameters are `S0=100`, `K=100`, `r=0.05`, `sigma=0.20`, `T=1`, with 100,000 antithetic pairs and RNG seed 42. The test suite checks the analytical reference, simulation agreement, deterministic reproducibility on the same implementation, expiry and zero-volatility edge cases, and invalid inputs.

## Assumptions and limitations

- Constant interest rate and volatility, lognormal terminal prices, no dividends, European exercise only.
- The result is **not** a forecast, trading recommendation, or calibrated market price.
- The PRNG seed is reproducible for a given C++ standard-library implementation, but the exact `std::normal_distribution` sequence is not portable across all implementations.
- Confidence intervals describe Monte Carlo sampling uncertainty **within the assumed model**, not model risk or real-world market uncertainty.
- Extreme parameter combinations may overflow exponentials; production numerical libraries need additional guards.

## Exercises

1. Increase the number of antithetic pairs and measure how the standard error scales approximately as `1/sqrt(N)`.
2. Add a European put and verify put–call parity within simulation uncertainty.
3. Implement a plain Monte Carlo estimator and compare its empirical variance with antithetic sampling across many seeds.
4. Extend the model to continuous dividend yield `q` and verify the analytical formula.

**AI disclosure:** This is an AI-assisted educational example, reviewed with compiled executable tests. No proprietary market data is included.

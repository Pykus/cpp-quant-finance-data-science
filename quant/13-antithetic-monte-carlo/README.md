# Lesson 13 — Antithetic Monte Carlo for European calls

**Learning goal:** reduce Monte Carlo sampling error by evaluating market scenarios in mirror-image pairs. Compare an ordinary and an antithetic option-price estimator at the **same payoff budget**; calculate standard errors correctly and cross-check against Black–Scholes.

If you are new to probability or finance, start with the [first-year student guide](BEGINNERS_GUIDE.md).

## The problem

A European call pays `max(S_T - K, 0)` at expiry. Our risk-neutral model generates hypothetical future share prices:

~~~text
S_T = S_0 * exp((r - 0.5 * volatility^2) * T + volatility * sqrt(T) * Z)
Y   = exp(-r * T) * max(S_T - K, 0)
~~~

Here `S_0` is today's share price, `K` the strike, `r` the annual continuously compounded risk-free rate, `T` time in years, and `Z` one standard normal draw. `Y` is a discounted payoff, and the model's option price is its expected value.

Write two estimators that each perform exactly **2N payoff calculations**:

1. **Independent:** draw 2N unrelated normal values; evaluate and average the 2N payoffs.
2. **Antithetic:** draw N unrelated normal values. For each `Z`, evaluate payoffs at `Z` and `-Z`, average the two, then average the **N pair means**.

The two outcomes in each antithetic pair are **dependent**. For the antithetic estimator, `N` pair averages — **not** `2N` payoffs — are the independent observations used to compute variance and standard error. A European call often benefits from negative covariance across each pair; this is not guaranteed for arbitrary payoffs.

## Build and run

Requirements: CMake 3.20+ and a C++20 compiler. No network calls or external dependencies.

From the repository root:

~~~sh
cmake -S quant/13-antithetic-monte-carlo -B build/quant-13 -DCMAKE_BUILD_TYPE=Release
cmake --build build/quant-13 --parallel
ctest --test-dir build/quant-13 --output-on-failure
./build/quant-13/antithetic_demo
./build/quant-13/antithetic_tests
~~~

Or from this lesson's folder use `cmake -S . -B build`, then `cmake --build build` and `ctest --test-dir build`. With multi-config generators (e.g. Visual Studio) executable paths include the configuration directory.

Representative output from one standard library (the exact Monte Carlo draws may differ):

~~~text
Black-Scholes reference: 10.450584
Independent MC: 10.405747 (SE 0.032805, CI 10.341452 .. 10.470043)
Antithetic MC: 10.431381 (SE 0.023153, CI 10.386002 .. 10.476760)
Independent observations: 200000 vs 100000; payoff evaluations per method: 200000
~~~

## Files and API

- [pricing.hpp](pricing.hpp) — input validation, risk-neutral simulated payoffs, fixed-seed generator, Welford running variance, confidence interval, analytic reference.
- [tests.cpp](tests.cpp) — deterministic assertions covering counts, mathematical invariants, reproducibility, invalid inputs and one observed variance reduction.
- [demo.cpp](demo.cpp) — executable demonstration.
- [CMakeLists.txt](CMakeLists.txt) — standalone CMake and CTest build.
- [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) — worked calculations and line-by-line explanation.

~~~cpp
using namespace quant::antithetic;
const EuropeanCall call{100.0, 100.0, 0.05, 0.20, 1.0};
const auto result = estimate(call, 100'000, 2026, Method::antithetic);
// result.price, result.variance, result.standard_error, result.low95, result.high95
~~~

The program calculates sample variance with denominator `n - 1` and reports a **normal-approximation** 95% interval `mean ± 1.95996398454 × standard_error`. It measures simulation error *within the model*, not uncertainty about market conditions.

The `std::mt19937_64` engine is deterministic for a seed. `std::normal_distribution` is not required to produce byte-for-byte identical draws across all standard-library implementations. Consequently the tests check pricing accuracy relative to estimated error, not hard-coded simulated outputs.

Inputs must be finite, with spot/strike/volatility/maturity strictly positive; a finite negative interest rate is allowed. Invalid counts or methods and detected arithmetic overflow are rejected. The simplified model omits dividends, market impact, jumps, stochastic volatility and early exercise.

**Complexity:** Both implementations run in `O(N)` time for 2N payoffs and keep only `O(1)` additional state. There is no heap allocation per simulated path.

**Follow-up exercise:** change the call payoff into a put payoff, preserving the correct observation count. Test several strikes and explain when the antithetic method does, or does not, reduce the standard error.

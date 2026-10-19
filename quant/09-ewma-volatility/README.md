# Lesson 09: Exponentially Weighted Volatility in C++20

EWMA is a small streaming model of time-varying return variance. This lesson connects a one-day conditional volatility forecast to the normal-model VaR and expected shortfall following Lesson 08 on historical tail measures. Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for step-by-step arithmetic and explanations.

## Update and interpretation

Use decimal returns: +2% means `0.02`. With decay lambda in `(0,1)`, previous variance `v_t`, and observed return `r_t`, the **next** forecast is:

    v_(t+1) = lambda * v_t + (1 - lambda) * r_t * r_t

Forecast volatility is the square root of variance. Low lambda responds quickly to shocks; high lambda retains older information. The implicit conditional mean of return is zero. With lambda `0.94`, weight half-life is about 11.2 observations.

## Compilation and tests

From the repository root:

    cmake -S quant/09-ewma-volatility -B build/ewma
    cmake --build build/ewma
    ctest --test-dir build/ewma --output-on-failure
    ./build/ewma/ewma_demo

Without CMake, in this lesson directory:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o ewma_tests
    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o ewma_demo
    ./ewma_tests
    ./ewma_demo

## A calculation by hand

With prior variance `0.0001`, lambda `0.94`, and observed return `+0.02`: `0.94*0.0001 + 0.06*(0.02)^2 = 0.000118`. New volatility is `sqrt(0.000118)`, or approximately 1.0863%. A return of `-0.02` produces the same squared shock.

## Tail-risk model

For a zero-mean Gaussian return with forecast sigma, the one-period normal-model 95% loss cutoff is `1.64485362695 * sigma`; expected shortfall in the worst 5% tail is `2.06271280751 * sigma`. With sigma 2% and portfolio value 10,000 the estimates are approximately 329 and 413 units. Unlike the historical quantiles from Lesson 08, these values rely on a strong distributional assumption.

## API and validation

`EwmaVolatility` exposes forecast variance, forecast volatility, decay, half-life, annualized volatility, processed observation count, and `observe(return)`. Invalid non-finite inputs or overflowing calculations raise exceptions **before** changing the saved state. `normal_tail_risk_95` returns both fractional and monetary VaR/ES.

The tests exercise arithmetic, input boundaries, return sign symmetry, zero returns, forecast chronology, parameter response, annualization, Gaussian constants, and state preservation after exceptions. No external library or real market data is needed.

## Limitations

The seed variance and decay factor must be chosen separately, preferably on earlier training observations. The normal model can underestimate severe losses when returns have heavy tails. Annualization with `sqrt(252)` assumes approximate scaling that need not hold for clustered volatility or serial dependence. The toy model ignores drifting means, correlations across assets, and calibration diagnostics. All included returns are synthetic.

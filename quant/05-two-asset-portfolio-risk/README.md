# Lesson 05: Measuring Risk in a Two-Asset Portfolio (C++17)

A portfolio's average return does not tell you how much its returns vary. Here we compute **sample standard deviation**, a simple measure of historical variability, for a portfolio containing two assets.

**Start here:** [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) teaches the mathematics and code from first principles, with a fully worked three-month example. Recommended prerequisites: [portfolio returns](../04-portfolio-returns/) and [covariance](../../statistics/02-covariance-correlation/).

## Run the example and the tests

From this directory, with a C++17 compiler:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o portfolio_risk
./portfolio_risk
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o portfolio_risk_tests
./portfolio_risk_tests
```

Expected example output:

```text
Mean portfolio return: 3.600%
Sample portfolio standard deviation: 2.884%
Sample covariance (A,B): -0.001200
```

The tests independently check the numerical example, the variance identity, exact-offset and single-asset cases, and rejected inputs; they print the number of checks passed.

## Inputs, outputs, and interpretation

`analyze_portfolio(returns_a, returns_b, weight_a)` takes two equal-length arrays of **paired decimal period returns** and a weight between 0 and 1. For example, `0.10` means `+10%`, and `0.60` means 60% invested in asset A. The result provides asset and portfolio means, sample variances, sample covariance, and portfolio standard deviation. Portfolio period returns are calculated as `w*A + (1-w)*B`.

This example uses three **synthetic monthly observations**. It assumes constant target weights (rebalanced each month), synchronized monthly returns, and at least two finite observations. Results are descriptive, not forecasts; the example excludes transaction costs, taxes, leverage, uncertainty estimates, and market impact. Standard deviation measures variation in returns, **not the full extent of investment risk**. Mixing daily and monthly series is invalid.

## Exercise

Run with `weight_a = 1.0`, `0.5`, then `0.0`. Does smaller standard deviation always mean higher expected return? Explain why covariance affects the result even if the individual asset variances stay unchanged.

# Quantitative Finance, Statistics & Data Science in C++

A beginner-friendly C++ learning repository covering quantitative finance, statistics, numerical methods, and data science.

**New to quantitative finance? Start with Lesson 00.** Each lesson builds on the previous one with plain-language explanations, worked examples, runnable C++ code, and exercises.

## Start learning: Quantitative Finance

1. **[Lesson 00 — Your first option calculation](quant/00-option-basics/)**: What is a call option? What are a strike price and payoff? Calculate simple examples in C++17. **No finance background needed.**
2. **[Lesson 01 — From option payoffs to Monte Carlo](quant/01-monte-carlo-option-pricing/)**: Why simulate possible future prices? Learn averages, model inputs, discounting, simulation uncertainty, and the Black–Scholes reference step by step.
3. **[Lesson 02 — Discounted cash flows and NPV](quant/02-discounted-cash-flows/)**: Understand the discounting step, calculate the present value of future payments, and test a small C++17 net-present-value calculator using synthetic examples.

4. **[Lesson 03 — Bond price and Macaulay duration](quant/03-bond-duration/)**: Bond prices, cash flows, and payment timing.
5. **[Lesson 04 — Portfolio returns](quant/04-portfolio-returns/)**: Weighted investment returns, with a full beginner's guide.
6. **[Lesson 05 — Two-asset portfolio risk](quant/05-two-asset-portfolio-risk/)**: Sample variability, covariance, and diversification, with a [beginner's guide](quant/05-two-asset-portfolio-risk/BEGINNERS_GUIDE.md).

7. **[Lesson 06 — Historical Sharpe ratio](quant/06-sharpe-ratio/)**: Compare monthly excess returns with sample variability using synthetic data and a [step-by-step beginner's guide](quant/06-sharpe-ratio/BEGINNERS_GUIDE.md).
8. **[Lesson 07 — Compounding and maximum drawdown](quant/07-compounding-and-drawdown/)**: Follow a fictional portfolio from one month to the next, calculate its worst fall from a past high, and learn the arithmetic with a [first-year student's guide](quant/07-compounding-and-drawdown/BEGINNERS_GUIDE.md).
9. **[Lesson 08 - Historical VaR and expected shortfall](quant/08-historical-var-and-expected-shortfall/)**: Measure historical tail losses using empirical percentiles, weighted tail averages, and a [worked beginner guide](quant/08-historical-var-and-expected-shortfall/BEGINNERS_GUIDE.md).

10. **[Lesson 09 — EWMA volatility forecasts](quant/09-ewma-volatility/)**: Forecast next-day volatility and a simplified dollar VaR from weighted returns, with a [beginner's guide](quant/09-ewma-volatility/BEGINNERS_GUIDE.md).

11. **[Lesson 10 — Black–Scholes implied volatility](quant/10-black-scholes-implied-volatility/)**: Invert observed European option prices using tested C++20 bisection, with a [first-year beginner's guide](quant/10-black-scholes-implied-volatility/BEGINNERS_GUIDE.md).

12. **[Lesson 11 — Binomial option trees](quant/11-binomial-option-trees/)**: Price European and American options with a risk-neutral tree and a [beginner's guide](quant/11-binomial-option-trees/BEGINNERS_GUIDE.md).
13. **[Lesson 12 — Greeks and delta hedging](quant/12-black-scholes-greeks/)**: Price European options, compute delta/gamma/vega, cross-check sensitivities and hedge a signed position; includes a [first-year beginner's guide](quant/12-black-scholes-greeks/BEGINNERS_GUIDE.md).

## Statistics and Data Science

1. [Descriptive statistics](statistics/01-descriptive-statistics/) — mean, median, and population variance.
2. [Covariance and Pearson correlation](statistics/02-covariance-correlation/) — paired samples and association.
3. [Simple linear regression](statistics/03-linear-regression/) — line fitting and interpreting a trend.
4. [Z-scores and standardization](statistics/04-z-score-standardization/) — sample spread and comparing relative positions.
5. [Standard error of the mean](statistics/05-standard-error-of-mean/) — uncertainty of an average under independent sampling, with a [first-year student guide](statistics/05-standard-error-of-mean/BEGINNERS_GUIDE.md).

6. [Bootstrap confidence intervals for the mean](statistics/06-bootstrap-mean-confidence-interval/) — use reproducible resampling to estimate uncertainty, with a [step-by-step beginners guide](statistics/06-bootstrap-mean-confidence-interval/BEGINNERS_GUIDE.md).

7. [Robust outlier detection using MAD](statistics/07-robust-outlier-detection/) — Screen unusual sensor readings with a median-based modified z-score; includes a [beginner guide](statistics/07-robust-outlier-detection/BEGINNERS_GUIDE.md).

## C++ Foundations

- [Fixed-width integers and safe conversions](lessons/01-types-and-overflow/) — integer types, checked conversions, and compile-time assertions.

## Systems Programming

1. [RAII file ownership](systems/02-raii-file-handling/) — tie an open file handle to object lifetime.
2. [Fixed-capacity ring buffer](systems/03-ring-buffer/) — FIFO storage with wraparound.
3. [Safe binary message codec](systems/04-binary-message-codec/) — a tested C++20 packet format with network byte order, bounds checks and typed messages.
4. [Bounded TCP stream framing](systems/07-tcp-stream-framer/) — reconstruct complete messages from fragmented or coalesced byte streams with fixed-capacity buffers, a beginner's walkthrough, and error-path tests.

## Other Topics

Statistics, probability, regression, time series, numerical computing, performance engineering, modern C++, Linux, embedded systems, and GPU computing.

The financial examples are educational and use simplified models; they are not investment advice or production trading systems.

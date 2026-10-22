# Lesson 12 — Black–Scholes Greeks and a First Delta Hedge

How much does an option price change when the share price moves by one unit? How does its exposure change as the market moves? This lesson answers those questions with a small, testable C++20 model.

## What you build

- Black–Scholes **European call and put** prices with continuous dividend yield.
- Analytic **delta**, **gamma** and **vega**.
- Independent central finite-difference estimates to check the formulas.
- A signed-contract delta hedge: the number of underlying shares that offsets the position's *current* first-order exposure.
- Deterministic unit tests covering textbook values, put-call parity, dividends, negative interest rates, parameter grids, and rejected inputs.
- A [first-year student's walkthrough](BEGINNERS_GUIDE.md) explaining the finance, calculus and code from first principles.

This is a teaching implementation; prices and inputs are illustrative. No live market data is needed.

## Compile and run

Requires a C++20 compiler, no third-party libraries:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o greeks_demo
    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o greeks_tests
    ./greeks_demo
    ./greeks_tests

With CMake (3.16 or later):

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure

## Mathematical model

Use spot S, strike K, annual continuously compounded rate r, annual continuous dividend yield q, annual volatility sigma > 0 and years until expiry T > 0.

    d1 = [ln(S/K) + (r - q + 0.5*sigma^2)*T] / (sigma*sqrt(T))
    d2 = d1 - sigma*sqrt(T)
    Call = S*exp(-q*T)*N(d1) - K*exp(-r*T)*N(d2)
    Put  = K*exp(-r*T)*N(-d2) - S*exp(-q*T)*N(-d1)
    Call delta = exp(-q*T)*N(d1)
    Put delta  = exp(-q*T)*(N(d1)-1)
    Gamma = exp(-q*T)*phi(d1) / (S*sigma*sqrt(T))
    Vega  = S*exp(-q*T)*phi(d1)*sqrt(T)

N is the standard normal cumulative distribution; phi is its density. Gamma and vega are the same for corresponding calls and puts in this model.

For S = K = 100, r = 0.05, q = 0, sigma = 0.20, T = 1:

| Quantity | Call | Put |
| --- | ---: | ---: |
| Theoretical price | 10.45058357 | 5.57352602 |
| Delta | 0.63683065 | -0.36316935 |
| Gamma | 0.01876202 | 0.01876202 |
| Vega, per unit of volatility | 37.52403469 | 37.52403469 |

A 0.01 *absolute* increase in volatility (20% to 21%) suggests a price change of about 0.375 before higher-order effects, not 37.5.

## Hedging example

A short position in one call representing 100 underlying shares has initial delta exposure

    -1 contract * 100 shares/contract * 0.63683065 = -63.683065 shares

Buying about 63.683065 shares sets the **instantaneous delta** to approximately zero. The delta changes when spot, time or volatility changes; this is not a risk-free portfolio, and gamma/vega remain.

The function accepts signed positions: positive for owned options, negative for sold options. It returns positive shares for a purchase and negative shares for a short sale.

## Checks and limitations

- Input prices, strikes, volatility and time must be strictly positive; all inputs must be finite. Negative interest rates and dividend yields are mathematically accepted.
- The model assumes European exercise, constant volatility and rates, continuous dividends, lognormal prices and frictionless continuous trading.
- The implementation intentionally excludes expiry and zero-volatility limits. Extreme magnitudes can cause floating-point underflow or ill-conditioned numerical derivatives.
- The finite-difference routines independently reprice perturbed inputs; they are cross-checks, not a replacement for convergence studies.
- It does not model transaction costs, bid/ask spreads, discrete dividends, early exercise, jumps or hedging P&L.

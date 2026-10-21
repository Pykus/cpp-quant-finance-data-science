# Lesson 11: Binomial Option Trees in C++20

Price European and American vanilla calls and puts with the Cox-Ross-Rubinstein (CRR) model. The tree evaluates possible stock prices step by step, then works backward to calculate the option's value today.

**Start with [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md)** if you are new to finance, probability or numerical C++. It derives the formulas, walks through a one-step option by hand, explains the implementation, and supplies exercises with answers.

## Features

- European calls and puts using discounted risk-neutral expectations.
- American exercise decisions at each lattice node.
- Constant continuously compounded interest rates and continuous dividend yields.
- An explicit deterministic path for zero volatility, including American early exercise.
- Independent European Black-Scholes benchmark for convergence checks.
- Numerically stable risk-neutral weights using std::expm1.
- Input validation, no-arbitrage probability checks and explicit overflow errors.
- One vector of option values: O(N) auxiliary storage, O(N^2) computation.
- Regression tests for prices, parity, exercise rights, convergence, invalid inputs and edge cases.

## Build and test

Requires a C++20 compiler and CMake 3.16+. No third-party dependencies, APIs or external data.

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/binomial_tests
    ./build/binomial_demo

Build without CMake:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o binomial_tests
    ./binomial_tests
    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o binomial_demo
    ./binomial_demo

For a Debug build, replace Release with Debug. Under GCC/Clang, enable runtime checks with -fsanitize=address,undefined and -fno-omit-frame-pointer.

## Example contract and output

| Input | Value |
| :--- | ---: |
| Spot / strike | 100 / 100 |
| Years to expiry | 1 |
| Risk-free interest rate | 5% |
| Annual volatility | 20% |
| Annual dividend yield | 0% |
| CRR steps | 400 |

The example executable prints values rounded to six decimals:

    European call:      10.445586
    Black-Scholes call: 10.450584
    European put:       5.568528
    American put:       6.088406

European tree pricing converges toward the continuous-time Black-Scholes value as the lattice is refined. The American put is more valuable here because early exercise may be optimal.

## What to read

- [binomial.hpp](binomial.hpp) - header-only contract API, validation, CRR recursion, American exercise and Black-Scholes reference.
- [main.cpp](main.cpp) - a reproducible demonstration using fictional contract values.
- [tests.cpp](tests.cpp) - assertions for the one-step hand calculation, option parity, exercise inequalities, numerical limits and invalid inputs.
- [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) - detailed first-year student's walkthrough.
- [CMakeLists.txt](CMakeLists.txt) - a portable C++20 build and CTest registration.

## Scope and numerical limitations

This is a financial mathematics exercise, not a live trading, pricing or execution service. Prices are synthetic; no market feeds are accessed. The assumptions include constant volatility, interest rates and proportional dividend yield, with no bid/ask spread, fees, discrete dividends or early-exercise trading costs.

A finite American tree permits exercise at discrete tree dates, rather than continuously. The CRR risk-neutral probability must lie in [0,1]; this can fail for excessively coarse trees or extreme combinations of drift and volatility. The implementation rejects these cases and allows at most 5000 steps. More steps improve the approximation but increase work quadratically. Black-Scholes is used **only for European options** as a reference.

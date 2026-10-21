# Lesson 11: Binomial Option Pricing From First Principles

## Learning objectives

This lesson starts from zero finance knowledge. You will learn to:

1. Explain a call, put, strike and option expiration date.
2. Calculate option payoffs from a few possible stock prices.
3. Compute an up/down stock-price tree from annual volatility.
4. Derive risk-neutral probabilities instead of memorizing a formula.
5. Discount future payments and work backward through a tree.
6. Identify the early-exercise choice of an American option.
7. Understand C++20 source, memory use, numerical checks and tests.
8. Compare a European tree price with Black-Scholes.

Prerequisites: percentages, basic algebra, and C++ variables and loops. There is no need for trading experience.

## What is an option?

Suppose a fictional stock costs **100** now. A call option gives you the *right*, but not the obligation, to purchase it at an agreed **strike price**, K, before or at an allowed date.

- When the future stock price is 120 and K=100, the call payoff is 20.
- When the future price is 80, you would not pay 100 to buy something worth 80. The call payoff is zero.
- A put is the *right to sell* at K instead. If the stock falls to 80, a put with K=100 pays 20.

Call payoff = **max(future stock price - K, 0)**.

Put payoff = **max(K - future stock price, 0)**.

| Future stock price | Call with K=100 | Put with K=100 |
| ---: | ---: | ---: |
| 70 | 0 | 30 |
| 90 | 0 | 10 |
| 100 | 0 | 0 |
| 110 | 10 | 0 |
| 140 | 40 | 0 |

Payoff is not profit. An option usually costs something to acquire today. Its eventual payoff is only one part of an investor's return.

## European and American exercise

The geographical names describe **when exercise is permitted**, not where the option is listed.

- European: the holder can exercise **only at expiration**.
- American: the holder may exercise at any allowed point up to expiration.

Under identical assumptions, the American option cannot be worth less: its holder can choose to wait and behave exactly like the European holder.

    American value >= European value

For a non-dividend-paying stock and a nonnegative interest rate, an American call normally has no benefit from early exercise. A deep-in-the-money American put can benefit from exercising early to receive the strike sooner. Dividend payments may also create an early-exercise incentive for American calls.

These statements are *constraints* that our tests can check.

## Why price options using a tree?

Real prices can move by many amounts. A binomial tree replaces that complicated process with a simple model: on each short interval, the stock goes either **up** or **down**.

A one-step example:

                 110 (up)
                /
       100 now
                \
                 90.909... (down)

Each branch is a hypothetical outcome, not a prediction that the stock must move to one of those exact prices.

Adding more small steps creates many final price possibilities. The model becomes a useful approximation while remaining understandable enough to calculate by hand.

A tree *recombines* when an up-then-down path reaches the same final stock price as down-then-up. The CRR model uses up and down multipliers whose product is one, making this possible.

## Reading the financial inputs

For one contract:

| Symbol | Meaning | Typical example |
| --- | --- | ---: |
| S0 | Stock price today (spot) | 100 |
| K | Option strike | 100 |
| T | Years to expiration | 1 |
| N | Number of small steps | 400 |
| dt | Years per step, T/N | 0.0025 |
| r | Annual continuously compounded risk-free rate | 0.05 |
| q | Annual continuous dividend yield | 0.00 |
| sigma | Annual volatility (standard-deviation scale) | 0.20 |

Decimals rather than whole percentages are used: 20% volatility is **0.20**, not 20.

The rate and dividend yield use a *continuous compounding* convention throughout this example. Mixing annual percentage yield conventions can produce inconsistent prices.

## Derive the up and down moves

In the Cox-Ross-Rubinstein (CRR) model:

    dt = T / N
    h = sigma * sqrt(dt)
    u = exp(h)
    d = exp(-h) = 1/u

Here "exp(x)" means e raised to x, and "sqrt(x)" means square root.

If the current price is S, then the next step has possible stock prices:

    Up price   = S * u
    Down price = S * d

For a concrete one-year, one-step example, choose u=1.1. Then d=1/1.1=0.9090909... . The volatility corresponding to this one-step move is sigma=ln(1.1).

A stock priced at 100 therefore moves to 110 or 90.909... in this simplified model.

After i steps with j upward moves:

    Price(i, j) = S0 * u^j * d^(i-j)

Since u*d=1, that is also:

    Price(i, j) = S0 * exp((2*j-i) * h)

The code uses the second form. It avoids multiplying long sequences of up and down factors.

## Derive the risk-neutral probability

How much should we weight the up outcome? We do not guess a real-world probability. Instead we use a **risk-neutral pricing weight** consistent with the model's financing assumptions.

Ignoring randomness, a stock with continuous dividend yield q has risk-neutral expected growth factor

    g = exp((r-q) * dt)

Let p be the risk-neutral weight on the up outcome and 1-p the weight on the down outcome.

The model requires:

    p * S*u + (1-p) * S*d = S*g

Divide both sides by S:

    p*u + (1-p)*d = g

Expand and rearrange:

    p*u + d - p*d = g
    p*(u-d) = g-d

Therefore:

    p = (g-d) / (u-d)

The down weight is **1-p**.

This p is a mathematical *pricing measure*, not a forecast of the chance that the stock will rise tomorrow.

An economically coherent one-step CRR model needs **0 <= p <= 1**. A very coarse grid or extreme model inputs may violate this. The implementation rejects such inputs rather than silently returning an invented price.

## Calculate a complete one-step call by hand

Use the following fictional parameters:

    S0 = 100
    K  = 100
    dt = 1
    r  = 0
    q  = 0
    u  = 1.1
    d  = 1/1.1

The pricing growth factor is g=exp(0)=1.

    p = (1-0.9090909)/(1.1-0.9090909)
      = 0.476190476...

Call payoff if the price goes up:

    max(110-100, 0) = 10

Call payoff if it goes down:

    max(90.909...-100, 0) = 0

Risk-neutral weighted payoff:

    p * 10 + (1-p)*0 = 4.76190476...

The annual rate is zero, so discounting does not change this result.

**One-step call price: approximately 4.761905.**

Now try the put:

- Up payoff: 0.
- Down payoff: 100-90.909...=9.090909...
- Weighted present payoff: (1-p)*9.090909... = 4.761905.

The put and call have the same price here because spot=strike and r=q=0. The program tests both values against these independently calculated results.

## Why discount future cash?

Money received in the future is not generally equivalent to money available now.

Under a continuously compounded annual rate r, the value today of a known payment X made dt years from now is

    Present value = X * exp(-r*dt)

At r=0.05 and dt=1:

    exp(-0.05) = approximately 0.951229

Thus a certain payment of 100 after one year has present value about 95.1229.

For an option node with known values at the next up and down nodes, the value of waiting is

    hold = exp(-r*dt) * (p*value_up + (1-p)*value_down)

The price of a European option at a node is exactly this continuation value.

For American options we have to consider an additional alternative: exercise immediately.

## Two steps and backward induction

A two-step recombining stock-price tree ends at three different prices:

    S0*u*u    (two up moves)
    S0*u*d    (one up and one down, in either order)
    S0*d*d    (two down moves)

There are four paths but only three unique final values, because u*d=d*u=1.

Follow these steps for a European call:

1. Compute the stock price at each final node.
2. Convert those three final prices into option payoffs.
3. On the preceding layer, compute discounted weighted payoffs from adjacent final nodes.
4. Repeat once more to calculate the option price at time zero.

This is called **backward induction**: start with final outcomes, where payoffs are known, and recursively infer earlier values.

A toy example with p=0.5, zero interest and final payoffs 0, 0, 12 gives:

- Lower predecessor = (0+0)/2 = 0.
- Upper predecessor = (0+12)/2 = 6.
- Start value = (0+6)/2 = 3.

With a nonzero rate, multiply each predecessor value by exp(-r*dt). Do not discount only at the very end unless you have explicitly accumulated the full discount factor.

## American exercise: choose the better alternative

An American option holder chooses between:

1. **Hold:** keep the option; continuation value comes from the discounted weighted next-step values.
2. **Exercise now:** take the immediate payoff at the current stock price.

The lattice uses the rule:

    value_at_node = max(immediate_exercise, hold)

For puts:

    immediate_exercise = max(K - stock_at_node, 0)

For calls:

    immediate_exercise = max(stock_at_node - K, 0)

For European exercise, the implementation skips the immediate-exercise comparison and uses only the continuation value.

This difference is encoded in a short conditional in binomial.hpp. It has major consequences for pricing American puts and dividend-paying American calls.

Finite-step American pricing evaluates exercise at the tree dates, not at every instant in continuous time. Refining the time grid improves the approximation, but costs additional computation.

## The zero-volatility edge case

What if volatility sigma equals zero?

The standard multipliers become u=d=1. The expression for risk-neutral p becomes 0/0. This is not a small numerical error: the ordinary two-branch construction is no longer defined.

Under the zero-volatility simplification, stock price follows one deterministic risk-neutral path:

    S(t) = S0 * exp((r-q)*t)

A European option can be valued by calculating its payoff at T and discounting to today.

An American option may be exercised at the current date or any subsequent allowed tree date. Compute the *present* exercise payoff at each date and take the maximum.

Example: put with S0=80, K=100, r=8%, q=0, sigma=0, T=1.

- Exercise now: receive 20.
- Wait: the stock price grows deterministically and the strike is paid later.
- In this example immediate exercise has the highest present payoff.

**American put value: exactly 20** in this deterministic tree. A regression test checks that result.

## Black-Scholes as an independent European reference

A fine CRR tree for a European vanilla option converges to the same lognormal continuous-time model priced by Black-Scholes.

With a continuous dividend yield, the European call formula is:

    Call = S0*exp(-q*T)*Phi(d1) - K*exp(-r*T)*Phi(d2)

where

    d1 = [ln(S0/K) + (r-q+sigma^2/2)*T] / [sigma*sqrt(T)]
    d2 = d1 - sigma*sqrt(T)

Phi(x) is the standard normal cumulative distribution function: the area of the bell curve to the left of x.

For the European put:

    Put = K*exp(-r*T)*Phi(-d2) - S0*exp(-q*T)*Phi(-d1)

The C++20 implementation calculates Phi using the standard math function std::erfc. This supplies a useful independent numerical reference.

For spot=strike=100, one year, r=5%, q=0, sigma=20%:

| Method | Value (approximately) |
| --- | ---: |
| Black-Scholes European call | 10.450584 |
| CRR 400-step European call | 10.445586 |
| CRR 400-step European put | 5.568528 |
| CRR 400-step American put | 6.088406 |

The American put can be higher than the European put because of early exercise. **Do not use the European Black-Scholes formula as an American put price.**

## Put-call parity: test more than one calculation

For European options with identical spot, strike, maturity and model assumptions:

    Call - Put = S0*exp(-q*T) - K*exp(-r*T)

This relationship is called *put-call parity*.

It creates a strong cross-check: independently priced calls and puts must satisfy the same identity for compatible assumptions.

For example, if r=q=0 and S0=K=100, then Call-Put=0, so the prices agree.

The test suite varies the number of steps and the dividend yield and verifies this relationship within a tight numerical tolerance.

The equality is **not generally a rule for American contracts**, because the exercise rights differ. When writing a parity test, make sure both contracts use European exercise.

## Read the C++20 contract structure

The API starts with a small type:

    struct Contract {
        double spot;
        double strike;
        double maturity_years;
        double annual_rate;
        double annual_dividend_yield;
        double annual_volatility;
        std::size_t steps;
        OptionKind kind;
        ExerciseStyle exercise;
    };

- A double represents fractional financial and mathematical quantities.
- std::size_t represents an unsigned container size or index.
- enum class gives named choices such as OptionKind::put and ExerciseStyle::american.
- A struct groups inputs that all describe the same fictional option.

The header exposes two main functions:

    price(contract)                  // CRR European or American value
    black_scholes_european(contract) // independent European benchmark

Their results are marked [[nodiscard]] so compilers may warn when a caller discards the result accidentally.

## Read the backward C++ loop

The option values live in one vector:

    std::vector<double> value(steps + 1);

At expiration the vector contains the payoff at every possible final stock price.

The outer loop travels toward the present:

    for (std::size_t i = steps; i > 0; --i) {
        for (std::size_t j = 0; j < i; ++j) {
            // Use value[j] and value[j+1] to price one earlier node.
        }
    }

The condition i>0 matters because std::size_t is unsigned: decrementing zero would wrap around.

Updates travel from the left of the vector to the right. value[j+1] is therefore still the *next-layer* successor when calculating the current node.

This reduces the storage requirement to **O(N)** values. The total backward-induction work is approximately N*(N+1)/2 operations, hence **O(N^2)** time.

For very large N, runtime grows quickly. This lesson deliberately caps N at 5000 instead of encouraging an accidental millions-of-nodes computation.

## A numerical detail: std::expm1

When x is extremely close to zero, calculating exp(x)-1 can lose accuracy: floating-point subtraction cancels leading digits.

std::expm1(x) computes that expression more carefully.

The risk-neutral p formula is

    p = [exp((r-q)*dt)-exp(-h)] / [exp(h)-exp(-h)]

Multiply numerator and denominator by exp(h):

    p = [exp((r-q)*dt+h)-1] / [exp(2*h)-1]

The code therefore calculates:

    p = std::expm1(drift+h) / std::expm1(2*h)

This is an algebraically equivalent formula with improved behavior for small h.

The code still validates p. Numerical stability never justifies pretending that a mathematically invalid probability is valid.

Inputs must also be finite; the header rejects NaN, infinity, nonpositive spot/strike/maturity, negative volatility, and out-of-range step counts. Arithmetic overflow throws instead of returning a misleading infinity.

## How to build, run and interpret tests

You need a C++20 compiler and CMake 3.16+.

From this lesson directory, run the following in your terminal:

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ctest --test-dir build --output-on-failure
    ./build/binomial_tests
    ./build/binomial_demo

CTest reports one **registered test program**. That executable runs many individual checks; it prints its own check and failure counts. Do not confuse the two levels.

To compile directly without CMake:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o binomial_tests
    ./binomial_tests

Important scenarios in tests.cpp:

- Textbook European call and put prices.
- A one-step call and put calculated by hand.
- Put-call parity across dividend yields and step counts.
- American >= European and American >= immediate payoff.
- Early-exercise premium for a put.
- No early-exercise premium for a non-dividend-paying call.
- Zero-volatility deterministic calculations.
- Convergence as the number of CRR steps increases.
- Tiny positive volatility, negative rates and overflow.
- Rejection of invalid inputs and invalid risk-neutral probabilities.

For a more defensive local build under GCC/Clang:

    g++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o checked_tests
    ./checked_tests

Sanitizers help detect memory and undefined-behavior errors. They are not a proof of mathematical or financial correctness.

## Where the model can go wrong

Every formula has assumptions. Make these explicit before using a model:

1. Stock moves are modeled as repeated multiplicative up/down factors.
2. Annualized volatility is constant over the life of the option.
3. Rates and continuous dividend yield are constant.
4. The pricing measure is risk-neutral, not a physical prediction.
5. Transaction costs, bid/ask spreads and execution delays are ignored.
6. Dividends are modeled as a continuous proportional yield, not scheduled cash distributions.
7. American exercise occurs at discrete tree dates.
8. Extreme inputs can violate no-arbitrage bounds or floating-point limits.

Numerical convergence does not imply market realism. A very accurate model price can still disagree with a market quote if assumptions are wrong.

The examples use fictional prices and parameters. They teach valuation logic, not trading strategies or promises of returns.

## Exercises with answers

Try each exercise before reading the answer.

**A. Payoff table.** K=50 and future stock prices 30, 50, 75. Find call and put payoffs.

Answer: calls 0, 0, 25; puts 20, 0, 0.

**B. Discounting.** What is the present value of 100 received in one year at a continuous annual 3% rate?

Answer: 100*exp(-0.03), approximately 97.0446.

**C. Risk-neutral p.** For u=1.25, d=0.8 and r=q=0, find p.

Answer: (1-0.8)/(1.25-0.8)=0.2/0.45=4/9.

**D. Flexibility.** Can an American put cost less than its identical European put in this model?

Answer: No. The American holder can choose the same maturity exercise date as the European holder.

**E. Programming.** Change the dividend yield in main.cpp to 0.03 and print European calls and puts.

Hint: verify Call-Put=S0*exp(-q*T)-K*exp(-r*T).

**F. Convergence.** Record call prices with 50, 100, 200 and 400 steps. Compare their errors against Black-Scholes.

Hint: convergence can oscillate for arbitrary neighboring step counts because of strike placement on the lattice. The test uses an even-step sequence for a specific at-the-money example.

**G. Edge case.** Explain why sigma=0 needs a separate path.

Answer: CRR gives u=d=1 and p would require dividing zero by zero. A deterministic stock path is the appropriate limit.

**H. Invariant.** Add a test that increases a put strike while holding everything else fixed.

Expected result: the higher-strike put should not be worth less.

## Recommended next steps

After you can calculate a one-step tree yourself, try these extensions:

1. Estimate delta by re-pricing after small spot changes.
2. Print the exercise boundary of an American put at each layer.
3. Study pricing error against Black-Scholes as N grows.
4. Derive a one-step stock-and-cash replicating portfolio.
5. Explore why discrete cash dividends require a different model.

For further study:

- John C. Hull, *Options, Futures, and Other Derivatives*: introductory option and binomial pricing chapters.
- Steven E. Shreve, *Stochastic Calculus for Finance I*: rigorous binomial asset pricing.
- C++ library references for std::vector, std::size_t, std::expm1 and std::erfc.

The most important habit is to check a simple case on paper before trusting a larger numerical program.

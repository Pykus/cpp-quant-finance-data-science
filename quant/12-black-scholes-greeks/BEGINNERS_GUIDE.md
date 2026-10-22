# A First-Year Student's Guide to Option Greeks and Delta Hedging

This chapter assumes you have written a few C++ programs but have never traded an option and have not studied calculus in depth. Work through the numerical examples with pencil and paper before running the code.

## 1. The question behind this lesson

Suppose a share currently costs 100. You have the right, but not the obligation, to purchase it for 100 at a specified future date. That right is a **call option**.

A model might value this option at 10.45 today. But "10.45" does not answer a risk question: what happens to that price if the share changes from 100 to 101?

Option traders therefore measure **sensitivities**. A sensitivity tells us how one quantity responds when another changes, while other inputs are held fixed. In finance these sensitivities are traditionally named with Greek letters.

You will study three:

1. **Delta** — response of the option price to a small change in the share price.
2. **Gamma** — response of delta itself to a change in the share price.
3. **Vega** — response of the option price to a small change in assumed volatility.

You will then combine delta with a signed number of option contracts to compute a basic hedge.

## 2. What are a call and a put?

A call provides the right to *buy* an asset at the **strike** K at expiry. A put provides the right to *sell* it at K. The value of the asset now is its **spot price** S.

For a European option the right can be exercised only at its expiry date. This is important: an American option can be exercised earlier and generally needs different valuation.

The cash received at expiry, before subtracting the premium initially paid, is called the **payoff**:

    call payoff = max(S_at_expiry - K, 0)
    put  payoff = max(K - S_at_expiry, 0)

Example with K = 100:

| Share price at expiry | Call payoff | Put payoff |
| ---: | ---: | ---: |
| 80 | 0 | 20 |
| 100 | 0 | 0 |
| 125 | 25 | 0 |

You cannot infer today's fair option price from only one possible future share price. We must account for many possible outcomes and for the time value of money.

## 3. A model is a set of assumptions, not a prediction

The Black–Scholes model uses a stylized probability model for share prices. For this lesson we assume:

- Trading is continuous and frictionless.
- The continuously compounded interest rate r is fixed.
- Volatility sigma (annualized variability of price returns) is fixed.
- The dividend yield q is paid continuously.
- Share prices follow a continuous lognormal diffusion.
- The option can only be exercised at expiry.

The model is useful as a mathematical benchmark even when these assumptions are unrealistic in practice.

Our C++ input structure uses these values:

    struct OptionInputs {
        double spot;
        double strike;
        double rate;
        double dividend_yield;
        double volatility;
        double maturity_years;
    };

**Units are crucial.** Twenty percent volatility is written 0.20, not 20. Three months is written 0.25 years. An annual continuously compounded rate of five percent is 0.05.

## 4. Interest and dividends: why discount?

If you invest 100 at a continuously compounded annual interest rate of 5%, after a year you have

    future amount = 100 * exp(0.05) = approximately 105.13

The reverse operation finds today's equivalent of a future payment:

    present value = future payment * exp(-r * T)

A dividend-paying share distributes part of its value over time. In this version of Black–Scholes we represent dividends by a **continuous yield** q and use

    discounted spot = S * exp(-q * T)

It is a model input, not a claim that companies really pay dividends every instant.

## 5. The normal curve

The standard normal distribution is a bell-shaped probability distribution centered at zero.

We use two mathematical functions:

- N(x), the cumulative distribution function (CDF): the probability that a standard normal variable does not exceed x.
- phi(x), the density (PDF): the height of the bell curve at x.

For example:

    N(0) = 0.5
    N(1) is approximately 0.8413
    phi(0) is approximately 0.3989

Do **not** confuse a probability with a density. The density can exceed a probability on an interval; it is not itself an event probability.

In C++, the complementary error function gives a stable CDF implementation:

    N(x) = 0.5 * erfc(-x / sqrt(2))

The PDF has the formula

    phi(x) = exp(-x*x/2) / sqrt(2*pi)

Read the functions named normal_cdf and normal_pdf in greeks.hpp and identify their equivalents.

## 6. Where d1 and d2 come in

The pricing formula groups inputs into two intermediate quantities:

    d1 = [ln(S/K) + (r - q + sigma^2/2)*T] / (sigma*sqrt(T))
    d2 = d1 - sigma*sqrt(T)

Here ln is the natural logarithm, exp is the exponential function, and sqrt is the square root.

Try the following numbers:

    S = 100, K = 100, r = 0.05, q = 0
    sigma = 0.20, T = 1

Because S/K = 1, ln(1) = 0. Also sigma^2/2 = 0.02. Therefore

    d1 = (0 + (0.05 + 0.02)*1) / (0.20*1) = 0.35
    d2 = 0.35 - 0.20 = 0.15

These standardized quantities let us use the normal curve in the option formulas. They are **not** direct predictions of next year's share price.

## 7. Prices of European calls and puts

Define the two discounted amounts:

    A = S * exp(-q*T)
    B = K * exp(-r*T)

Then

    call price = A * N(d1) - B * N(d2)
    put price  = B * N(-d2) - A * N(-d1)

For the numerical case in section 6, N(0.35) is about 0.63683 and N(0.15) is about 0.55962.

    call = 100 * 0.63683 - 100*exp(-0.05)*0.55962
         = approximately 10.45

The more accurate implementation returns about 10.45058357 for the call and 5.57352602 for the put.

Why is the call more valuable in this example? Positive interest rates make paying the strike in the future less costly in present-value terms. This is an intuition, not the complete explanation of option pricing.

## 8. Delta: the first sensitivity

Imagine a price function C(S), meaning "the call price as a function of the current share price."

Its **delta** is the derivative dC/dS. If you have not yet learned derivatives, think of it as a ratio over an *extremely small* share-price change:

    delta is approximately
      (new option price - old option price) / share price change

For a European option in our model:

    call delta = exp(-q*T) * N(d1)
    put delta  = exp(-q*T) * [N(d1) - 1]

If q = 0, delta of a call lies between 0 and 1. Delta of a put lies between -1 and 0.

For our call:

    call delta = N(0.35) = 0.63683065

For a share-price increase of 1, the call price might increase by approximately 0.6368 **if every other input stays fixed**. The approximation deteriorates for larger moves because delta itself changes.

For the put:

    put delta = N(0.35) - 1 = -0.36316935

A rising share price generally makes a put less valuable.

A crucial identity is

    call delta - put delta = exp(-q*T)

This gives us a useful independent test.

## 9. Gamma: how fast delta changes

If delta tells us the slope of option price against spot, gamma tells us how quickly that slope changes.

In calculus notation:

    gamma = d(delta)/dS = d^2(option price)/dS^2

In Black–Scholes:

    gamma = exp(-q*T) * phi(d1) / [S*sigma*sqrt(T)]

For our example:

    gamma = approximately 0.01876202

If S rises by 1 and all else is fixed, delta might change by roughly +0.0188. Gamma is positive for plain European long calls and puts with these assumptions.

Notice that calls and puts with equal S, K, r, q, sigma and T have the **same gamma**.

Gamma tells us why a hedge that was neutral at S = 100 will no longer be neutral after spot changes. The portfolio must be rebalanced to remain delta-neutral.

The formula contains 1/sqrt(T) and 1/sigma. Near expiry or with tiny volatility, sensitivities can change very rapidly around the strike. Numerical estimation also becomes more delicate.

## 10. Vega: sensitivity to volatility

Volatility sigma describes the model's assumed annual variability of returns. Vega measures price change when we vary this volatility input:

    vega = d(option price)/d(sigma)

For the same European options:

    vega = S*exp(-q*T)*phi(d1)*sqrt(T)

Our example produces:

    vega = approximately 37.52403469

The size looks alarming until you check the units. The derivative is measured for a **full 1.0 change** in sigma.

A change from 0.20 to 0.21 is only 0.01, so:

    approximate option price change = 37.52403469 * 0.01
                                    = 0.37524035

Both the call and put have the same vega under these assumptions. Vega is positive for long vanilla European calls and puts when sigma and T are positive.

Volatility is not the same thing as tomorrow's share-price direction. Higher volatility increases the modeled potential for extreme outcomes in either direction.

## 11. Numerical differentiation: test mathematics with mathematics

Analytic derivatives are efficient and often accurate, but an implementation may still contain a sign error or missing discount factor.

We can independently approximate delta by repricing at nearby spot prices:

    h = S * 0.0001
    numerical delta = [price(S+h) - price(S-h)] / (2*h)

This is called a **central finite difference**. It uses a price above and below today's price.

For gamma we use

    numerical gamma = [price(S+h) - 2*price(S) + price(S-h)] / h^2

The gamma calculation is particularly sensitive to rounding when h is too small because we subtract nearly equal numbers.

For vega, use a small absolute volatility bump v:

    numerical vega = [price(sigma+v) - price(sigma-v)] / (2*v)

The source code computes these checks by calling the price function repeatedly. It does *not* calculate delta or gamma by simply returning the analytic result again.

**Numerical differences are approximations, not exact equality.** A good test uses realistic tolerances and a parameter grid rather than demanding bit-for-bit equality.

## 12. Put-call parity: another independent check

For a European call and put sharing all model inputs:

    call price - put price
      = S*exp(-q*T) - K*exp(-r*T)

This identity links the prices without asking the option pricer to reproduce a known hard-coded answer.

With q = 0, S = K = 100, r = 0.05 and T = 1:

    right-hand side = 100 - 100*exp(-0.05)
                    = approximately 4.87705755

The actual example gives:

    call - put = 10.45058357 - 5.57352602
               = approximately 4.87705755

The same equality holds at other strikes, interest rates and dividend yields within the model.

Try to compute this identity before running the test executable.

## 13. From delta to a hedge

Suppose you **sell one call option contract**. For this example, one contract represents 100 underlying shares.

One long call has delta about +0.63683 per underlying share. A sold call has the opposite sign:

    option position = -1 contract
    multiplier = 100 shares per contract
    total option delta = -1 * 100 * 0.63683065
                       = -63.683065 shares of exposure

To make the combined first-order delta approximately zero, buy 63.683065 actual shares:

    option exposure + share exposure
      = -63.683065 + 63.683065 = 0

That is the value returned by shares_for_zero_delta.

Its formula is

    shares = -signed_contract_count * multiplier * option_delta

The code chooses a clear sign convention:

- Positive signed_contract_count: an option you own.
- Negative signed_contract_count: an option you sold.
- Positive returned shares: buy shares.
- Negative returned shares: sell/short shares.

**Delta-neutral does not mean risk-free.** Delta changes as spot changes (gamma), option values move when volatility changes (vega), and options lose time value as expiry approaches. Theoretical continuous hedging also ignores commissions, bid/ask spreads and execution uncertainty.

If the share market allows only whole shares, rounding creates additional residual exposure. This teaching example deliberately keeps fractional shares in the arithmetic.

## 14. C++ code tour

The implementation is in greeks.hpp. It is a header-only lesson so that the example and tests can compile separately without introducing a package manager.

### Types

OptionInputs keeps model assumptions together, making it more difficult to accidentally swap two arguments when calling functions. OptionKind is an enum class, not an untyped string.

OptionGreeks returns price, delta, gamma and vega in one value. Callers do not need to compute four separate functions and accidentally use different inputs.

### Validate before calculating

The formulas divide by spot, volatility and the square root of time. Invalid zeros or negative values would break the model.

validate rejects NaN, infinity and values outside the documented domain. We use exceptions because invalid model inputs represent a caller error; the function should not quietly return a plausible-looking zero.

### Compute shared intermediate values once

The code computes d1 and d2, discounted spot and strike, then branches for call or put. Gamma and vega are shared formulas for both.

The normal CDF uses std::erfc from the standard C++ library. No external probability library is needed.

### Modern C++ details

- const prevents accidental reassignment of intermediate quantities.
- [[nodiscard]] asks callers to notice returned prices or hedge sizes.
- enum class keeps call and put names scoped and strongly typed.
- noexcept on the normal functions indicates they are not designed to throw.
- The code uses no raw owning pointers or manual allocation.
- Tests use exceptions for an informative failure message.

Notice that noexcept does **not** mean a function cannot produce non-finite values: validation still matters in the full pricing function.

## 15. Compile, run and read the output

From this lesson directory:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o greeks_demo
    ./greeks_demo

Then compile and run the independent checks:

    g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests.cpp -o greeks_tests
    ./greeks_tests

Alternatively use CMake and CTest as explained in README.md.

The tests deliberately vary rates (including a negative rate), dividend yields, strikes, volatilities and maturities. They also test invalid inputs and the signs of hedge positions.

## 16. Exercises — stop before reading the answers

**Exercise A.** A short call has delta 0.40. One contract represents 100 shares. How many shares should you buy to neutralize its delta?

**Exercise B.** You own two puts, each with delta -0.25 and multiplier 100. How many shares are required for a delta-neutral portfolio?

**Exercise C.** Vega is 24 per unit volatility. If volatility moves from 30% to 32%, estimate the first-order option price change.

**Exercise D.** Why is the finite-difference gamma usually more sensitive to a tiny h than finite-difference delta?

**Exercise E.** For two options with the same strike, maturity and market parameters, what is call delta minus put delta if q=0? What if q=0.02, T=1?

**Exercise F.** Make a copy of the example inputs and set dividend_yield to 0.03. Recompile. What changes and why?

**Exercise G.** Change the option position in main.cpp from -1 to +1. Predict the sign of the hedge *before* running the program.

## 17. Answers and reasoning

**A:** Buy 40 shares, because -1 * 100 * 0.40 = -40 delta exposure.

**B:** Buy 50 shares. Owned puts contribute 2 * 100 * (-0.25) = -50 share-equivalents of delta; a long position in 50 shares offsets that negative exposure.

**C:** Approximately 24 * 0.02 = 0.48 in the option price, before nonlinear effects.

**D:** The numerator subtracts two times the center value from two nearby prices. It is of order h squared, so round-off can dominate when h becomes too small.

**E:** For q=0, the difference is exactly 1 in the model. For q=0.02 and T=1, it is exp(-0.02), approximately 0.98019867.

**F:** Continuous dividends reduce the spot's discounted contribution and change both d1 and d2. The call price and delta generally decline relative to q=0 when other inputs stay fixed.

**G:** For a long call, the option delta is positive, so the offsetting share position is negative.

## 18. Where to go next

To extend this lesson:

1. Add theta (time sensitivity) and rho (interest-rate sensitivity) and check them using finite differences.
2. Write a discrete-rebalancing simulation in synthetic price paths and compute its residual P&L.
3. Compare hedge errors for a call and a put when the share moves by different amounts.
4. Measure finite-difference convergence as h shrinks: do not assume the smallest h gives the most accurate answer.
5. Extend the API to handle expiry and zero volatility as explicit limiting cases.

The mathematical formulas are derived for a simplified model. Do not treat the example hedge as a recommendation to transact in live markets.

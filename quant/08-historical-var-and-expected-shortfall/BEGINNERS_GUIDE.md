# Beginner's Guide: Historical VaR and Expected Shortfall

This guide assumes only school-level percentages and some beginner C++.
No economics course, investment account, or previous statistics course is needed.
Every observation below is **invented for learning**.

## 1. Why measure losses?

Suppose you have a fictional investment portfolio. Its value changes daily.
Some days the value goes up. Other days it goes down.

An average return says something about the *typical* experience.
But it can hide rare, very bad days. For that reason, we often
look at the **loss tail**: the worst part of the observations.

In this lesson you will answer two different questions:

1. What loss sits at a chosen high percentile of the historical observations?
2. How severe were the losses in the worst part of those observations?

The first question leads to **Value at Risk (VaR)**.
The second leads to **Expected Shortfall (ES)**.

These are historical descriptions, not crystal balls or promises.

## 2. From portfolio value to a return

A *simple return* is:

~~~text
return = (ending value - starting value) / starting value
~~~

Imagine a portfolio starts at 100 and ends at 94.

~~~text
return = (94 - 100) / 100
       = -6 / 100
       = -0.06
       = -6%
~~~

The decimal number `-0.06` is what the program receives.

If the same portfolio rises from 100 to 105, its return is:

~~~text
(105 - 100) / 100 = +0.05 = +5%
~~~

Keep percentages and decimal fractions separate:

| Percentage | Decimal return |
| --- | ---: |
| -10% | -0.10 |
| -4% | -0.04 |
| 0% | 0.00 |
| +3% | +0.03 |

The code uses decimals, not whole percentage points.
Passing `-4.0` means **-400%**, an invalid simple return,
and is correctly rejected.

## 3. Convert returns into losses

The convention in this lesson is:

~~~text
loss = -return
~~~

Why do this? It gives us one consistent sign convention:
**the larger the loss number, the worse the outcome**.

| Return | Signed loss |
| ---: | ---: |
| -10% | +10% |
| -6% | +6% |
| 0% | 0% |
| +3% | -3% |
| +5% | -5% |

A gain becomes a negative *signed loss*.
This may look surprising, but it prevents hidden changes
to the underlying mathematical quantity.

Do not confuse a signed loss with a price or a probability.

## 4. Sort the observations

Take these ten synthetic daily returns:

~~~text
-10%, -6%, -4%, -2%, 0%, +1%, +2%, +3%, +4%, +5%
~~~

Turn each return into a loss:

~~~text
+10%, +6%, +4%, +2%, 0%, -1%, -2%, -3%, -4%, -5%
~~~

Then sort losses from *smallest* to *largest*:

| Position (1-based) | Sorted loss |
| ---: | ---: |
| 1 | -5% |
| 2 | -4% |
| 3 | -3% |
| 4 | -2% |
| 5 | -1% |
| 6 | 0% |
| 7 | +2% |
| 8 | +4% |
| 9 | +6% |
| 10 | +10% |

Positions 9 and 10 are the two worst daily losses in this sample.

Notice that sorting deliberately discards the *order of days*.
We are studying the distribution of losses, not their chronology.

This is different from **maximum drawdown** in Lesson 07:
the sequence in which values rise and fall matters for drawdown.

## 5. What is a percentile?

A percentile is a position in sorted observations.
A high loss percentile is close to the worst observed values.

Our implementation uses the **nearest-rank** convention:

~~~text
rank = ceil(confidence * observation_count)
~~~

Here `ceil` means *round UP to the next whole number*.
The selected position is one-based.

At 80% confidence with 10 observations:

~~~text
rank = ceil(0.80 * 10)
     = ceil(8)
     = 8
~~~

Look up position 8 in the table. It is **+4%**.

That is our **historical VaR at 80% confidence**.

In plain English: in this empirical sample, 80% of the
observed losses are at or below 4%.
It does **not** mean tomorrow's loss cannot exceed 4%.

Different statistical libraries can choose different
sample-quantile conventions. Be explicit about your choice.

## 6. The difference between VaR and ES

VaR picks a threshold. It does **not** describe how bad
the observations beyond the threshold were.

Two portfolios could share a 4% VaR, even if one has a worst
loss of 6% and the other has a worst loss of 40%.

**Expected Shortfall** addresses this by averaging
the observations in the worst part of the sample.

At 80% confidence, the worst fraction is:

~~~text
tail fraction = 1 - confidence
              = 1 - 0.80
              = 0.20
~~~

For 10 observations:

~~~text
tail mass = 0.20 * 10 = 2 observations
~~~

The two worst observed losses are:

~~~text
+10%, +6%
~~~

Average them:

~~~text
ES = (10% + 6%) / 2
   = 8%
~~~

Therefore our sample has:

| Statistic | Result |
| --- | ---: |
| Historical VaR, 80% | 4% |
| Historical ES, 80% | 8% |

The ES is larger because it focuses on worse outcomes.

## 7. What happens at 85% confidence?

Confidence does not always give a whole number of observations.

~~~text
tail mass = (1 - 0.85) * 10
          = 0.15 * 10
          = 1.5 observations
~~~

We cannot literally observe half a day.
Instead, the **empirical-quantile integral** convention used here
puts a fractional weight on the next boundary observation.

- Take the worst loss, +10%, with weight 1.0.
- Take the second-worst loss, +6%, with weight 0.5.
- Divide by total weight 1.5.

~~~text
ES = (1.0 * 10% + 0.5 * 6%) / 1.5
   = (10% + 3%) / 1.5
   = 8.666666...%
~~~

At 85% confidence nearest-rank VaR is:

~~~text
rank = ceil(0.85 * 10)
     = ceil(8.5)
     = 9
VaR = loss at position 9 = 6%
~~~

At 85%, **VaR = 6%** and **ES = about 8.67%**.

This fractional observation is a *weight in a calculation*,
not a claim that a half-observation occurred in reality.

## 8. The important 95% small-sample trap

Imagine estimating a 95% loss statistic from ten observations.

~~~text
tail mass = (1 - 0.95) * 10 = 0.5 observation
~~~

The resulting ES uses half the weight of the single worst loss,
so it equals that worst observation in this implementation.

Does this mean we understand rare 95% tail events from ten days?
Absolutely not. It merely exposes how little evidence we have.

For real risk analysis, consider sample length, regime changes,
stress testing, data quality, dependencies, and how uncertainty
around tail estimates might be assessed.

## 9. Translate a percentage loss into money

Suppose our fictional portfolio is worth 10,000 currency units
at the beginning of the chosen period.

For a fixed portfolio size:

~~~text
money loss = signed loss fraction * portfolio value
~~~

At 80% confidence:

~~~text
VaR money = 0.04 * 10,000 = 400
ES money  = 0.08 * 10,000 = 800
~~~

The portfolio value must be positive and finite.

This multiplication is a **simple scaling convention**.
It does not model deposits, withdrawals, position changes,
liquidity constraints, or losses greater than available capital.

## 10. A gentle reading of the C++ code

Open [risk.hpp](risk.hpp).

### 10.1 A non-owning view: std::span

~~~cpp
std::span<const double> simple_returns
~~~

This parameter means: "a read-only view of consecutive doubles."

The function does not take ownership of the caller's data.
It can accept an `std::array` or `std::vector`
without allocating a second input container.

The caller must keep the underlying data alive while the
function is working. The function does not store the span.

### 10.2 Reject bad inputs

~~~cpp
if (!(confidence > 0.0 && confidence < 1.0)) {
    throw std::invalid_argument("...");
}
~~~

The condition rejects 0, 1, negative values, infinities,
and NaN (not-a-number). For NaN, both comparisons are false.

A simple return cannot be smaller than -100%:
a portfolio value cannot go below zero without additional
leverage or liability assumptions outside our model.

### 10.3 Convert returns to signed losses

~~~cpp
losses.push_back(-value);
~~~

The input is read but never modified.
We transform into a separate vector that we can sort safely.

### 10.4 Pick the empirical quantile

~~~cpp
std::ceil(confidence * count)
~~~

In C++, vectors use **zero-based indexing**:
the first element has index 0.

Our explanatory table has **one-based positions**:
the first observation has rank 1.

That is why the implementation subtracts 1 after the
nearest-rank computation.

### 10.5 Compute the weighted worst tail

We visit losses from largest to smallest.
At each step we consume up to one observation's weight:

~~~cpp
const double weight = std::min(1.0, remaining);
es += (weight / tail_mass) * (*it);
remaining -= weight;
~~~

If the remaining mass is 0.5, only half of the next
observation enters the average.

Scaling each contribution by total weight produces
the same weighted average as dividing the sum at the end.

### 10.6 Why not use noexcept?

This function allocates a vector and can throw
`std::bad_alloc` or an input-validation exception.

Marking it `noexcept` would be incorrect:
an exception would terminate the program.

The `[[nodiscard]]` annotation, however, is useful:
ignoring the calculated risk is likely a programming mistake.

## 11. Run a real demonstration

With a working C++20 compiler:

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic main.cpp -o demo
./demo
~~~

You should see:

~~~text
Observations: 10
Confidence: 80.00%
Historical VaR: 4.00%
Historical expected shortfall: 8.00%
VaR at 10000 currency units: 400.00
ES at 10000 currency units: 800.00
~~~

Inspect [tests.cpp](tests.cpp) to learn how we check:
different confidences, reordering, ties, zero returns,
single observations, and malformed data.

## 12. Common misunderstandings

**Misunderstanding A:** "80% VaR means an 80% chance of losing 4%."

No. The figure is a *loss threshold* at an empirical
percentile, not a probability of exactly that loss.

**Misunderstanding B:** "VaR tells me my maximum future loss."

No. Observed worst losses can exceed VaR, and future
losses can be even larger than anything in the sample.

**Misunderstanding C:** "Expected shortfall is a prediction."

Not here. It is a weighted average of *historically observed*
losses in the worst part of the sample.

**Misunderstanding D:** "Risk statistics are independent of
sampling choices."

No. Daily and monthly returns describe different horizons.
Sampling periods, missing data, and portfolio changes matter.

**Misunderstanding E:** "A negative VaR is a bug."

No. If all observed returns were positive, their signed
losses were all negative. We preserve the signed convention.

**Misunderstanding F:** "Historical risk is the same as
maximum drawdown."

No. VaR and ES sort returns, ignoring day order. Drawdown
depends on the path of portfolio values over time.

## 13. Exercises: calculate before running

### Exercise 1: Another confidence level

Use the ten-return example. Calculate nearest-rank VaR
and weighted ES at 90% confidence.

### Exercise 2: Change the order

Reverse the return list. Predict whether VaR and ES
change at 80% confidence. Explain your reasoning.

### Exercise 3: Two equal worst losses

Use these five returns:

~~~text
-5%, -5%, 0%, +1%, +2%
~~~

At 60% confidence, calculate VaR and ES.

### Exercise 4: A positive-only sample

Use these four returns:

~~~text
+1%, +2%, +3%, +4%
~~~

At 50% confidence, find signed VaR and ES.
Why could both be negative?

### Exercise 5: Check invalid data

Predict whether each input will be accepted:

- Confidence 1.0 and returns `[-0.05, 0.02]`.
- Confidence 0.8 and no returns.
- Confidence 0.8 and returns `[-1.01, 0.02]`.
- Confidence 0.8 and returns `[-1.00, 0.02]`.

### Exercise 6: Larger portfolio

The 80% VaR is 4% and ES is 8%.
Scale to a fictional starting portfolio value of 25,000.

### Exercise 7: Think like an analyst

A 99% ES from just ten observations equals the worst
observed loss. Give two reasons not to treat this as a
trustworthy prediction of a future rare loss.

## 14. Answers, with reasoning

**Exercise 1:** Rank = ceil(0.90 * 10) = 9.
VaR = the ninth sorted loss = 6%.
Worst tail mass = 1 observation, so ES = 10%.

**Exercise 2:** Neither changes. Sorting gives the same
multiset of losses regardless of the original day order.

**Exercise 3:** Losses sorted are -2%, -1%, 0%, 5%, 5%.
Rank = ceil(0.60 * 5) = 3, so VaR = 0%.
The worst 2 observations are 5% and 5%, so ES = 5%.

**Exercise 4:** Signed losses sorted are -4%, -3%, -2%, -1%.
Rank = ceil(0.50 * 4) = 2, so VaR = -3%.
The worst 2 signed losses are -1% and -2%.
ES = (-1% - 2%) / 2 = -1.5%.
These are not negative amounts of money owed;
they represent gains under the signed-loss convention.

**Exercise 5:** Reject; reject; reject; accept.
The final observation `-1.00` is precisely a 100% loss.

**Exercise 6:** VaR = 0.04 * 25,000 = 1,000.
ES = 0.08 * 25,000 = 2,000 currency units.

**Exercise 7:** Ten observations rarely represent a changing
market environment adequately, and an unobserved extreme
future loss may be much worse than the observed maximum.
You could also mention data errors, changing portfolio exposure,
market liquidity, or tail estimate uncertainty.

## 15. Where to go next

To connect this lesson to earlier topics:

1. Lesson 04 explains what a return is and how it aggregates.
2. Lesson 05 teaches variability and portfolio risk.
3. Lesson 06 teaches the Sharpe ratio (mean vs variability).
4. Lesson 07 teaches compounding and maximum drawdown.
5. Lesson 08 (this lesson) focuses on the worst part of returns.

You can next compare **parametric VaR** under a normal
distribution with **historical VaR**, and investigate why
fat-tailed real data can challenge simple normal models.

The central lesson: always state the *definition* of a
risk statistic and validate its *sample and assumptions*
before interpreting the numerical result.
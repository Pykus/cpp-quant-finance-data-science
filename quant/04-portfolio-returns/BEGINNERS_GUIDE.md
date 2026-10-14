# Beginner's Guide: Portfolio Returns from First Principles

## 1. Why calculate a portfolio return?

Suppose you save money and divide it between two investments. One gains value while the other loses value. Looking at either investment alone cannot tell you whether your *total savings* increased. A portfolio return answers this question.

A **portfolio** is a collection of investments. An **asset** is something you own that has a value. A **return** describes how much the value changed relative to the starting value. We use a fixed **period** such as one month or one year; all returns must refer to the same period.

## 2. The only mathematics you need

A percentage means 'out of 100'. Ten percent is `10 / 100 = 0.10`; minus five percent is `-5 / 100 = -0.05`. When multiplying or adding returns in formulas, use these decimal forms.

For one asset:

`return = (ending value - starting value) / starting value`

If an asset grows from 200 to 220 units of currency, the gain is `220 - 200 = 20`, and the return is `20 / 200 = 0.10 = 10%`.

If it falls from 200 to 190, the return is `(190 - 200) / 200 = -0.05 = -5%`. A negative return means a loss.

## 3. What is a portfolio weight?

A **weight** is the fraction of the portfolio's starting value invested in one asset.

Suppose you start with 1,000 currency units. You put 600 in asset A and 400 in asset B.

- Weight A: `600 / 1000 = 0.60 = 60%`.
- Weight B: `400 / 1000 = 0.40 = 40%`.
- Check: `0.60 + 0.40 = 1.00`.

The weights sum to one because all starting money has been allocated. We use *starting* weights when measuring the return for this period.

## 4. A complete numerical calculation

Assume A gains 10% and B loses 5% over the same year.

1. Starting value of A: 600.
2. A's change: `600 * 0.10 = +60` currency units.
3. A's ending value: `600 + 60 = 660`.
4. Starting value of B: 400.
5. B's change: `400 * (-0.05) = -20` currency units.
6. B's ending value: `400 - 20 = 380`.
7. Portfolio ending value: `660 + 380 = 1040`.
8. Portfolio gain: `1040 - 1000 = 40`.
9. Portfolio return: `40 / 1000 = 0.04 = 4%`.

So the total portfolio gained 4%, **not** 5% (the simple average of +10% and -5%). The investments did not receive equal amounts of money.

## 5. Derive the weighted-average formula

The portfolio's gain is the sum of each investment's gain:

`gain = starting_A * return_A + starting_B * return_B`.

Divide both sides by total starting capital `S`:

`portfolio_return = (starting_A / S) * return_A + (starting_B / S) * return_B`.

Each ratio `starting_i / S` is a weight `w_i`. Therefore:

`portfolio_return = w_1 * r_1 + w_2 * r_2 + ... + w_n * r_n`.

For our example:

`0.60 * 0.10 + 0.40 * (-0.05) = 0.06 - 0.02 = 0.04 = 4%`.

This formula is a **weighted average**: larger investments contribute more to the total result.

## 6. How the C++ program matches the mathematics

In `main.cpp`, `weights` contains fractions such as `0.60` and `0.40`; `returns` contains decimal returns such as `0.10` and `-0.05`.

The loop calculates `weights[i] * returns[i]` for each asset and adds the products to `result`. The program checks that both vectors have the same nonzero length, values are finite, weights are nonnegative and weights sum to one. The example multiplies the decimal result by 100 **only for display** as a percentage.

## 7. Common mistakes

- Using `10` instead of `0.10` for a 10% return. That would represent a 1,000% return.
- Averaging asset returns without considering how much money is invested in each.
- Mixing monthly returns for one asset with annual returns for another.
- Using ending weights instead of starting weights.
- Assuming the example predicts future performance. It only calculates a past or hypothetical period.
- Treating a positive return as proof that an investment is safe. Return alone does not describe risk.

## 8. Exercises and worked solutions

**Exercise A.** Invest 200 in X and 800 in Y. X gains 20%, Y gains 5%. Find the weights and portfolio return.

**Solution:** Weights: `200/1000=0.20`, `800/1000=0.80`. Weighted return: `0.20*0.20 + 0.80*0.05 = 0.04 + 0.04 = 0.08 = 8%`. Check in currency: gains `40 + 40 = 80`; `80/1000=8%`.

**Exercise B.** Invest equally in two assets. One gains 12%, the other loses 8%. What happens?

**Solution:** Each weight is `0.50`. `0.50*0.12 + 0.50*(-0.08) = 0.06 - 0.04 = 0.02 = 2%` gain.

**Exercise C.** An investment of 750 grows to 810. What is its return?

**Solution:** Gain is `810 - 750 = 60`. Return is `60/750 = 0.08 = 8%`.

## 9. Where to go next

Once weighted returns are clear, study how returns vary from period to period. **Variance** measures the spread of observed values; **covariance** describes how two series move together. Those concepts help explain why combining investments may change portfolio risk.

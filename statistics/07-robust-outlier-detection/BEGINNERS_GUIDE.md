# Beginner's guide: robust statistics for noisy measurements

## 1. The real problem

Suppose you maintain a refrigerator, server room, or sensor network. You receive daily temperatures:

`9.8, 10.0, 10.1, 10.2, 9.9, 10.0, 35.0`

A reading of 35°C looks suspicious. You want a **repeatable screening rule**, not a decision to delete it automatically.

## 2. Median versus mean

The mean adds every value and divides by the number of values; a single very large measurement raises it. The median sorts values and chooses the middle:

`9.8, 9.9, 10.0, 10.0, 10.1, 10.2, 35.0`

There are seven values, so the fourth is the **median = 10.0°C**.

For an even number of values, average the two middle values.

## 3. Compute absolute deviations

Subtract the median from every original value and ignore the sign:

`0.2, 0.0, 0.1, 0.2, 0.1, 0.0, 25.0`

Sort them:

`0.0, 0.0, 0.1, 0.1, 0.2, 0.2, 25.0`

The middle distance is **MAD = 0.1°C**. The huge measurement does not inflate MAD nearly as much as it would inflate a standard deviation.

## 4. Score each measurement

The modified z-score for 35°C is

`0.67448975 × (35 - 10) / 0.1 = approximately 168.62`.

This exceeds **3.5**, so row 7 is flagged for **human review**. We don't claim the reading is definitely erroneous.

For 9.8°C:

`0.67448975 × (9.8 - 10.0) / 0.1 = approximately -1.35`.

This ordinary reading is not flagged.

## 5. What if MAD is zero?

Take `5, 5, 5, 5, 99`. The median is 5 and the median absolute deviation is 0. Dividing by 0 is not meaningful. A naive program might produce infinity or falsely classify the 99. Our implementation instead returns:

- `scores_defined = false`
- no outlier flags
- an empty `std::optional<double>` for every score

This means **the method cannot decide**, not that all observations are safe. You may need a separate domain-specific rule or a different robust estimator.

## 6. Why `std::span` and `std::optional`?

A C++20 `std::span<const double>` references existing readings without copying them. It also prevents the function from overwriting observations. An `std::optional<double>` makes the distinction between a valid score of zero and an **undefined** score explicit.

## 7. Practice

1. Find the median and MAD of `1, 2, 3, 4, 100`.
2. Explain why this tool produces identical flagged indices if you add 10°C to every reading.
3. Double all measurements. Why do modified z-scores stay the same?
4. Lower the threshold from 3.5 to 0.5. What kind of false alarms might you create?
5. Design a separate operational rule for the `MAD = 0` situation. Explain its trade-offs.

## 8. Caveats

The 3.5 threshold is a **heuristic**. It is not a p-value, an error probability, or a universal definition of an outlier. Repeated timestamps, drifting sensors, and multiple operating regimes may require different treatment.

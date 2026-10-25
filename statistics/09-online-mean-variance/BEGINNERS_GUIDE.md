# Beginner's guide — streaming statistics

## A concrete example

For the eight fictional readings:

    2, 4, 4, 4, 5, 5, 7, 9

Their sum is 40, so the average is 5. The squared distances from 5
sum to 32: population variance is 32/8 = 4 and sample variance is
32/7 ≈ 4.5714.

The distinction matters: use the population formula when these are
*all* the readings that define the population; use sample variance
to estimate variability in a larger population under ordinary sampling
assumptions.

## Why a running algorithm?

Keeping all readings costs memory proportional to the data set.
Welford updates three numbers as each new value arrives.

- `count_` records how many samples have been processed.
- `mean_` stores the average computed so far.
- `m2_` stores the sum of squared deviations, not variance itself.

As new readings arrive, a small adjustment is applied to the mean.
The second difference uses the **updated** mean, preserving numerical
stability compared with subtracting huge squares.

## Combining batches

Suppose you calculate moments from two separate files. Their means
may differ; you cannot just add their two `M2` values. You must
account for the difference between batch means. The code's `merge`
method applies the correction weighted by both batch sizes.

## Practical cautions

Input `NaN` and infinity are rejected. No algorithm based on finite
`double` can safely represent *all* possible magnitudes or counts.
A singleton has no unbiased sample-variance denominator.
No synchronization is provided: a `RunningStats` instance is not safe
to mutate concurrently from multiple threads. Merge separately computed
batches after they finish instead.

## Exercise

Implement a running standard-deviation method and compare its result
with manually calculated cases. Add a test where two batches have
different means but the same within-batch variance.

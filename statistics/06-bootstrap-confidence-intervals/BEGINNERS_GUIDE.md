# First-year student's guide: bootstrap confidence intervals

## An everyday question

A fictional call center observes these waiting times (in minutes):

```text
3, 5, 4, 7, 6, 9
```

The **sample mean** is (3+5+4+7+6+9)/6 = **5.67 minutes**. The **sample median** is the midpoint of the sorted values 3,4,5,6,7,9: (5+6)/2 = **5.50 minutes**. Those two numbers describe exactly the **six calls you saw**.

But there are thousands of other calls you did **not** observe. The unknown average of the larger population may differ from 5.67. Our question is: *how uncertain are we about that population average?*

## Why the bootstrap helps

Suppose it is expensive to collect more calls. The **bootstrap** treats the observed sample as a small stand-in for the population.

Draw six entries **with replacement** from the observed six. With replacement means you can draw the same observation twice:

```text
Observed:   3  5  4  7  6  9    mean = 5.67
New draw A: 5  5  3  6  9  5    mean = 5.50
New draw B: 9  9  7  4  6  9    mean = 7.33
New draw C: 3  4  3  3  5  6    mean = 4.00
```

Repeat this **2,000 times**, recording the mean of each draw. Each drawn sample has the **same size** as the original six. A valid draw may contain six copies of the same number, though this is uncommon.

The resulting 2,000 numbers form a **bootstrap distribution of the sample mean**. We can do the same with medians instead of means.

## Where the 95% interval comes from

Sort all 2,000 computed means. The **2.5th percentile** cuts off the lowest ~2.5% of the values; the **97.5th percentile** cuts off the highest ~2.5%. The central 95% is the **percentile bootstrap confidence interval**.

For the C++ implementation we interpolate between neighboring sorted values when the requested percentile position is not an integer. Interpolation avoids an arbitrary jump when moving between two adjacent observations.

The exact endpoints depend slightly on the pseudorandom draws, but the input mean is always the same: **5.67 minutes**.

## What it means, and what it does NOT mean

A 95% confidence interval describes how the **method** behaves if you repeat the whole experiment many times, drawing new independent samples from the true population. Under suitable assumptions, about 95% of intervals generated in this way cover the fixed population parameter.

It does **not** say that 95% of individual waiting times lie in the reported interval. Nor does it say there is a 95% probability that a fixed true average lies in our already-computed range.

With only six original observations, bootstrap intervals may be very inaccurate. This lesson illustrates the method; do not make operational staffing decisions from six calls alone.

## Average versus median

The mean reacts strongly to one unusually long call. The median describes the middle typical call and is usually more robust to extreme values. Each measures something different.

Our C++ function accepts `stats::Statistic::mean` or `stats::Statistic::median`. The **resampling algorithm stays the same**, only the statistic computed for each draw changes.

## How the C++20-style code works in C++17

`std::vector<double>` stores the original observations. `const std::vector<double>&` avoids copying the input on every function call. `std::mt19937_64` generates reproducible pseudorandom values, and `std::uniform_int_distribution<std::size_t>` selects a valid index from 0 to n-1.

Each iteration fills a reusable vector of n measurements, computes its mean or median and appends the result to another vector. `std::sort` orders all resampled statistics before calculating the percentile endpoints.

The function returns a small `stats::Interval` structure: point estimate, lower bound, upper bound, confidence level and number of resamples.

For invalid inputs the function throws an exception. Important validations include at least two measurements, finite numbers (not NaN or infinity), and confidence between 0 and 1.

**Reproducibility detail:** fixed seed + same C++ library give repeatable results. Standard C++ does not guarantee that different implementations of `std::uniform_int_distribution` will produce identical index sequences.

## Practice problems

1. Replace 9 minutes with **30 minutes**. Does the mean or the median change more? Why?
2. Change confidence from **0.95 to 0.90**. Does a narrower interval make intuitive sense?
3. Increase the number of resamples from **2,000 to 4,000**. Does that give you four times as many *real observations*? (No.)
4. Try `{5,5,5,5,5,5}`. Predict the bootstrap interval before running the program.
5. Why might daily stock returns violate the assumption of independent, identically distributed observations? Research block bootstrap methods.

## Summary

One sample produces one measured average. A bootstrap simulates repeated sampling **from that sample** to estimate uncertainty. It is useful, approachable and flexible—but only as trustworthy as the representativeness and dependence assumptions behind the original observations.

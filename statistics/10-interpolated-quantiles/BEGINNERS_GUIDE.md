# Beginner's guide — interpreting percentiles

## What does a percentile mean?

A percentile is a location within an ordered data set. The median is
the 50th percentile. Here a type-7 convention is used because there are
multiple accepted ways to calculate percentiles for finite samples.

For [1, 2, 3, 4], the 25th percentile falls 25% of the way through
the three **gaps** between the four ordered values:
(4 - 1) × .25 = .75, which is between the first and second numbers.
Interpolating gives 1 + .75 × (2 - 1) = 1.75.

## Read the C++ code

- Copy the input vector to protect the caller's order.
- `std::sort` arranges values from low to high.
- `std::floor` finds the lower index, and `std::ceil` the upper.
- The fractional part becomes a weight between those adjacent values.
- `interquartile_range` calls the same routine for 25% and 75%,
  then subtracts the results.

## When should I use it?

Descriptive summaries, box plots and exploratory anomaly detection
can all use quantiles. A percentile is **not** a confidence interval
and does not automatically describe uncertainty about a population.

The sample can contain duplicates and negative values. Empty input
does not have a percentile. Streaming data may call for approximate
quantile sketches rather than sorting an entire history.

## Your turn

Compare two data sets that have the same median but very different
interquartile ranges, and explain what that tells you about their spread.

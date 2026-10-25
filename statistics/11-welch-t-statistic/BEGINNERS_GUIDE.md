# Beginner's guide — comparing two averages

Suppose two independent groups have different sample averages.
Is the difference large compared with their ordinary variability?

A t-statistic expresses the difference in units of **estimated
standard error**. A difference of 1 with standard error 0.5 gives
t=2. With error 2, the same difference gives t=0.5.

Welch's approach does not assume both population variances are equal.
It combines each group's estimated variance scaled by its sample size.

## Reading the code

- `std::span<const double>` borrows a contiguous sequence without
  copying it, and prevents modifying values through the view.
- `summarize` computes sample mean and variance in one pass.
- `aa` and `bb` are the two sample-variance contributions
  to the squared standard error.
- The denominator of the degrees-of-freedom formula accounts for
  both group sizes and unequal variance contributions.
- `std::sqrt` converts squared error into standard error.

## Do not overinterpret

A t-statistic is not a probability. We deliberately stop before
computing a p-value or declaring "significance." Real decisions need
a documented research question, sampling plan, and uncertainty limits.

## Exercise

Calculate results for two tiny samples by hand. Then double both
sample sizes by adding comparable independent observations and
discuss why standard error often shrinks.

# Bootstrap confidence intervals: from zero to C++17

## 1. Why estimate uncertainty?

Suppose a factory makes thousands of parts. We measure only five:
**8, 10, 12, 14, 16** (all fictional measurements).
We want the **population mean**, the average of *all* the parts.
Measuring everyone would be expensive. So we use a **sample**.
The five observed values are the sample, and their average is the
**sample mean**. The actual population mean remains unknown.

Calculate the sample mean by hand:
8 + 10 + 12 + 14 + 16 = 60.
The sample size is n = 5. Divide: 60 / 5 = **12**.

An answer of "12" alone does not tell us how reliable the estimate is.
A new random sample of five parts might yield 11 or 13 instead.
The mean varies across samples. That is **sampling variability**.

## 2. Terminology without assuming statistics knowledge

- **Population**: every member of the group about which we ask.
- **Sample**: the members we actually observe.
- **Observation**: one measured number.
- **Sample size n**: count of observed numbers (here n=5).
- **Statistic**: quantity calculated from the sample, such as its mean.
- **Parameter**: quantity describing the population, generally unknown.
- **Point estimate**: a single guess for a parameter, such as 12.
- **Interval estimate**: two endpoints summarizing plausible uncertainty.
- **Sampling distribution**: distribution of a statistic across many
  hypothetical independent samples from the same population.
- **Bootstrap**: approximate that distribution by reusing observed data.

## 3. The central trick: sample with replacement

Imagine writing five observed numbers onto five slips of paper.
Pick one slip, write down the number, **put the slip back**,
mix the slips and pick again. Repeat until you have five numbers.
This creates one **bootstrap resample** of size five.

For example, we might obtain:
8, 8, 10, 16, 16.

The arithmetic is (8 + 8 + 10 + 16 + 16) / 5 = 58 / 5 = **11.6**.
Another valid resample might be [8,8,8,10,10]:
(8+8+8+10+10)/5 = 44/5 = **8.8**.
We have not measured new parts. We reused existing observations.

**Why put each slip back?** If we always pick without replacement,
drawing five numbers from exactly five slips would just permute them.
Every resample mean would be 12; that cannot capture uncertainty.

Keep the bootstrap resample size the same as the original sample size:
we are approximating the variability of **the original five-item mean**.
Changing the resample size changes that target.

## 4. Bootstrap distribution of the mean

Repeat the previous process, say **10,000** times.
Each resample has five observations.
Calculate **one mean** from each resample.
You now have 10,000 sample means.

Some simulated means will be small; some will be larger.
Their spread approximates the sampling variability of the mean,
under the assumption that the observed distribution resembles
the population distribution.

The program stores all these means in `std::vector<double>`.
It sorts them from smallest to largest before finding percentiles.

## 5. What exactly is a percentile?

In a sorted list, a percentile indicates a position.
The **50th percentile**, or median, is the midpoint.
The 2.5th percentile is near the lower end of the list.
The 97.5th percentile is near the upper end.

For a **95% central interval**, we leave about 2.5% of
the resampled means outside each end.
Our endpoints are therefore the **2.5th and 97.5th percentiles**.

For an 80% central interval, discard 10% at each end.
The central 80% interval is generally **narrower** than the 95% one
when computed from the same list of resampled means.

The code uses the interpolation convention:
position = (B - 1) * percentile_probability,
where B is the number of resampled means.
When the position falls between two indices, it interpolates.

## 6. Work a small interpolation example

Imagine five **hypothetical** already sorted resample means:
[8, 10, 12, 13, 15]. We use only five to illustrate arithmetic,
not as a serious attempt at a confidence interval.

For the 10th percentile:
position = (5 - 1) * 0.10 = 0.4.
It lies 40% of the way between 8 and 10:
8 + 0.4*(10 - 8) = **8.8**.

For the 90th percentile:
position = (5 - 1) * 0.90 = 3.6.
It lies 60% of the way between 13 and 15:
13 + 0.6*(15 - 13) = **14.2**.

The corresponding 80% *central* interval in this toy example
is [8.8, 14.2]. Ten thousand resamples provide more stable
percentile estimates than five, but cannot fix a tiny original sample.

## 7. Bootstrap standard error explained

The **raw-data standard deviation** measures variation among
individual parts. The **standard error of the sample mean**
measures how much the estimated mean varies across samples.

An analytical approximation from the previous lesson was:
SE = sample_standard_deviation / sqrt(n).

We can also calculate an SE directly from the bootstrap distribution:
take the sample standard deviation of the **10,000 bootstrap means**.
That number is what the `bootstrap_standard_error` field reports.

The two SE estimates are generally similar in scale, but need not
agree exactly, especially when n is small.

## 8. Interpreting "95% confidence" carefully

A population parameter, such as the true population mean,
is a fixed value under this frequentist interpretation.
The interval changes when we take a different sample.

If a method achieves 95% coverage, then when it is repeated
over many independent samples, around 95% of its intervals
contain the fixed population parameter.

That does **not** mean there is a new 95% probability that
the fixed parameter lies in this specific already calculated interval.
A simple percentile bootstrap does not guarantee perfect
coverage, so treat the advertised level as a target.

## 9. Follow the C++ program step by step

**Validation** checks for at least two observations, a confidence
strictly between 0 and 1, finite values, and a reasonable
number of resamples. "Finite" excludes NaN and infinity.

**Original mean** is computed by summing in `long double`
before dividing by n; this helps reduce intermediate rounding.

**Random index**: `std::mt19937_64` generates a pseudorandom
sequence. `std::uniform_int_distribution<std::size_t>` chooses
an index from 0 through n-1. Every draw can select an index
selected before, implementing replacement.

**Simulated mean**: for each iteration, draw n indices and average
their observed values. Save the resulting simulated mean.

**Spread**: a stable running update calculates variance of the
simulated means. This is a Welford-style calculation.

**Interval**: sort all simulated means, compute the two requested
percentiles, and return both endpoints plus the SE and original mean.

## 10. C++17 features worth recognizing

- `const std::vector<double>&`: read a collection without copying.
- `std::size_t`: suitable for container sizes and indices.
- `std::uint64_t`: fixed-width integer seed.
- `[[nodiscard]]`: encourages callers to use the return value.
- `std::invalid_argument`: meaningful failure on bad input.
- `std::mt19937_64`: a pseudorandom engine, not cryptography.
- `std::uniform_int_distribution`: samples valid integer indices.
- `std::vector::reserve`: avoids repeated growth allocations.
- `std::sort`: sorts resampled means for percentile extraction.
- `long double`: extra intermediate arithmetic precision.

A fixed seed makes results reproducible in one C++ standard-library
implementation, which is useful for regression tests.
Other C++ standard libraries may implement distribution sampling
differently even when using the same engine and seed.

## 11. Time and space costs

If n is sample size and B is the number of resamples, then
drawing and averaging takes about B*n steps.
Sorting B bootstrap means costs approximately B*log(B) work.
The program stores B means, so it uses O(B) extra memory.

The 10,000-replicate demonstration is a small workload.
Millions of repeats cost time and memory and still cannot
manufacture information missing from the original sample.

## 12. When this method can mislead you

**Dependent data:** measurements along a time series, including
many financial returns, are often autocorrelated.
Independent point resampling destroys that dependence.
A *block bootstrap* preserves neighboring groups instead.

**Selection bias:** if you only measure unusually large parts,
bootstrapping those parts does not recover the small ones.

**Five observations:** great for learning, usually too few
for strong real-world inference. A large B does not increase n.

**Very skewed data or heavy tails:** simple percentile intervals
may have poor actual coverage. Look into BCa or studentized
bootstrap as advanced alternatives.

**Constrained statistics:** estimates close to a hard boundary
may need methods designed for that situation.

## 13. Exercises (try them before looking at answers)

1. Calculate the mean of [8,8,10,16,16] by hand.
2. Explain why drawing *without* replacement yields no spread
   when every original observation appears once.
3. Predict mean, lower/upper endpoints and SE for [7,7,7,7].
4. Explain why an 80% interval is usually narrower than a 95%
   interval built from the same simulated means.
5. Add 100 to every observation. What happens to the mean,
   interval endpoints and standard error?
6. Double all five values. What happens?
7. Give two examples of real measurements that might depend
   on their neighboring observations.
8. Why can't more bootstrap repetitions cure selection bias?
9. Why is a fixed random seed useful in automated testing?
10. Explain the difference between raw-data SD and bootstrap SE.

## 14. Answers with reasoning

1. Sum is 58 and n=5, so 58/5 = **11.6**.
2. Every draw has exactly the same collection, just in a
   different order. Their arithmetic means are identical.
3. Every resample consists only of 7s. Mean and both endpoints
   are 7. The bootstrap SE is zero.
4. Central 80% discards 10% per tail; central 95% discards
   only 2.5% per tail, preserving more extremes.
5. Mean and interval endpoints move up by 100; SE is unchanged.
6. Mean, interval endpoints and SE all double for positive scaling.
7. Consecutive daily stock returns and repeated readings
   from a slowly warming sensor are examples.
8. Repeatedly reusing the same biased observations still
   represents the same biased selection.
9. It makes a failed result repeatable during debugging.
10. SD describes individual values; SE describes an estimator's
    variability across different hypothetical samples.

## 15. Next steps

Compare bootstrap SE with analytical SE from the previous lesson.
Then study actual coverage with simulated populations, Student's
t intervals, BCa bootstrap, and dependence-aware block resampling.
Do not confuse computational repetitions with independent new data.

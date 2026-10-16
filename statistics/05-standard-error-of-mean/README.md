# Lesson 05: Standard error of the mean in C++17

**Question:** How variable might our calculated average be if we repeatedly
took new random samples from the same population?

Start with [the first-year student's guide](BEGINNERS_GUIDE.md). The previous
lesson introduced [sample standard deviation and z-scores](../04-z-score-standardization/).
Here we use that standard deviation for a **different purpose**: estimating
how much the sample *mean* would vary from sample to sample.

## Essential words

- **Observation:** one measured value.
- **Population:** all possible values in the group of interest.
- **Sample:** the values actually collected.
- **Sample size (`n`):** the number of observations.
- **Sample mean (`xbar`):** all observations added and divided by `n`.
- **Deviation:** one observation minus its sample mean.
- **Sample variance (`s^2`):** squared deviations summed, then divided by `n-1`.
- **Sample standard deviation (`s`):** square root of sample variance; it
  describes differences among individual observations.
- **Standard error of the mean (`SE`):** estimated standard deviation of
  the *sampling distribution of the mean*; `SE = s / sqrt(n)`.
- **Sampling distribution:** the distribution of a statistic across many
  hypothetical independent samples from the same population.

The standard error is **not** the standard deviation of the raw observations.

## Worked numerical example (fictional sensor readings)

The independent sample is `8, 10, 12, 14, 16`.

| Value | Difference from mean 12 | Squared difference |
| ---: | ---: | ---: |
| 8 | -4 | 16 |
| 10 | -2 | 4 |
| 12 | 0 | 0 |
| 14 | 2 | 4 |
| 16 | 4 | 16 |

Mean: `(8+10+12+14+16)/5 = 12`.

Squared deviations sum: `16+4+0+4+16 = 40`.

Sample variance: `40/(5-1) = 10`.

Sample standard deviation: `s = sqrt(10) = 3.162277660...`.

Standard error of mean: `SE = sqrt(10)/sqrt(5) = sqrt(2) = 1.414213562...`.

Both SD and SE use the original units. SE describes uncertainty in the
*estimated mean under sampling assumptions*, not uncertainty in a particular
individual reading.

## Build, run, and test

Use a C++17 compiler. From this lesson's directory:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
    standard_error.cpp main.cpp -o mean_demo
./mean_demo
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
    standard_error.cpp tests.cpp -o mean_tests
./mean_tests
```

Expected output:

```text
Observations: 5
Sample mean: 12.000000
Sample standard deviation: 3.162278
Standard error of the mean: 1.414214
```

The executable `mean_tests` prints its total successful checks.
No third-party libraries, generated market data, or network services are needed.

## Source files

- `standard_error.hpp`: declares the result fields and computation interface.
- `standard_error.cpp`: validates finite inputs, calculates running mean
  and sample spread using Welford's update, and divides SD by `sqrt(n)`.
- `main.cpp`: runs the synthetic five-reading worked example.
- `tests.cpp`: checks known numerical values, shifting/scaling, constant
  readings, large offsets, and invalid input handling.

## Assumptions and limitations

The textbook formula `s / sqrt(n)` estimates uncertainty of the mean when
observations are independent and comparably sampled from a stable population,
with finite variance. Correlated time-series measurements may require a
different method. Convenience samples and systematic bias remain biased even
if `n` is large. SE is **not** itself a 95% confidence interval; interval
construction also needs a sampling model and an appropriate critical value.
Do not infer accuracy or trading performance from these fictional numbers.

## Try it yourself

1. Compute the mean, sample SD and SE for `10,14`.
2. Why does multiplying all values by two also multiply SE by two?
3. What happens to SE for four identical observations?
4. Explain why collecting the same reading four times does not necessarily
   produce four independent observations.

The guide explains these exercises and the code step by step.

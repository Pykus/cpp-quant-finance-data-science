# Beginner's guide — computing spread one measurement at a time

## What is the business question?

You operate an API. Some customers complain about slow requests. You measure four response times: **10, 12, 14, 16 milliseconds**. The average is **(10+12+14+16)/4 = 13 ms**. But an average by itself does not show how much observations vary.

Subtract 13 from each observation: **−3, −1, +1, +3 ms**. Square these differences: **9, 1, 1, 9 ms²**. Their sum is **20 ms²**.

- Population variance divides by n: **20/4 = 5 ms²**.
- Sample variance divides by n−1: **20/3 ≈ 6.6667 ms²**. This adjustment accounts for estimating the mean from the same sample.
- Sample standard deviation is the square root: **sqrt(20/3) ≈ 2.582 ms**, back in milliseconds.

Use population variance to describe exactly the values you have. Use sample variance when treating them as a sample of a wider process, with suitable assumptions.

## Welford's algorithm

Rather than retain every response time, keep **n**, **mean**, and **M2** (the sum of squared deviations). On receiving x:

1. Increment n.
2. Compute delta = x − old_mean.
3. Update mean = old_mean + delta/n.
4. Update M2 = old_M2 + delta × (x − new_mean).

| New reading (ms) | n | Current mean | M2 |
| ---: | ---: | ---: | ---: |
| 10 | 1 | 10 | 0 |
| 12 | 2 | 11 | 2 |
| 14 | 3 | 12 | 8 |
| 16 | 4 | 13 | 20 |

At the end, divide M2 by n or n−1 to obtain population or sample variance.

## Why is the one-pass method important?

A naive variance formula subtracts large quantities such as **sum(x²)** and **sum(x)² / n**. Consider readings near **1,000,000,000,000**, with differences of 1–4 units. Floating-point rounding can wipe out the tiny difference between those large quantities. Welford tracks deviations around a running mean. The tests include this case.

C++ uses floating-point numbers with finite precision. The implementation uses **long double** for intermediate calculations, but precision still varies by machine. Even a stable algorithm cannot recover information rounded away in the original input.

## Combining two workers

Suppose server A sees **10, 12** and server B sees **14, 16**. Each independently computes n=2, M2=2 and means 11 and 15.

Define delta = meanB − meanA = 4. The merged total is:

- n = nA + nB = 4.
- mean = meanA + delta × nB/n = 11 + 4×2/4 = 13.
- M2 = M2A + M2B + delta² × nA×nB/n = 2+2+16×4/4 = 20.

The merged summary matches a single pass through the four values, without transferring their raw data. This is called a **mergeable accumulator**.

## Things that often go wrong

1. Confusing population and sample variance.
2. Reporting zero for sample variance with one reading; it is **undefined**. The method returns an empty optional.
3. Inserting NaN or infinity. This implementation rejects such values.
4. Sharing one accumulator across threads without a mutex. Use one per worker and merge after synchronization.
5. Treating standard deviation as a guarantee about future observations. It is a descriptive statistic, not a forecasting model.

## Exercises

1. Hand-calculate sample variance of [2, 4, 6] using the table above, then add a test.
2. Split [10, 12, 14, 16] into differently sized partitions and test merging.
3. What happens if all 100 readings equal 42?
4. Why is sample variance undefined for only one observation?
5. Does a single extreme outlier change the answer much? Is standard deviation robust?

## References

Welford (1962), Note on a Method for Calculating Corrected Sums of Squares and Products. Chan, Golub, and LeVeque (1983), Algorithms for Computing the Sample Variance.

# Statistics Lesson 01: Mean, Median, and Variance in C++17

**Goal:** Learn the three basic ways to summarize a dataset. No previous statistics knowledge required.

## 1. What do these words mean?

Imagine five measurements: **2, 4, 4, 6, 9**. A **mean** is their arithmetic average. A **median** is the middle measurement after sorting. **Variance** measures spread: how far observations typically lie from the mean, using squared distances.

## 2. Calculate by hand

**Mean:** Add the values: 2 + 4 + 4 + 6 + 9 = 25. Divide by five observations: **5**.

**Median:** Sort the observations (already sorted here). The third of five values is **4**. For an even number of values, average the two middle ones.

**Population variance:** Subtract the mean (5) from each observation, then square each difference:

| Value | Difference | Square |
| ---: | ---: | ---: |
| 2 | -3 | 9 |
| 4 | -1 | 1 |
| 4 | -1 | 1 |
| 6 | 1 | 1 |
| 9 | 4 | 16 |

Sum the squares: 9 + 1 + 1 + 1 + 16 = 28. Divide by five: **5.6**. This is **population variance**, treating all five observations as the complete population. **Sample variance** instead divides by n - 1 when estimating population variance from a random sample.

## 3. Compile and run

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o stats
./stats
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o stats_tests
./stats_tests
```

Expected program output: `mean=5 median=4 population_variance=5.6`. Tests exit successfully without printing text.

## 4. Understand the code

- `std::vector<double>` stores the observations. Passing by value means sorting does not modify the caller's data.
- `std::sort` orders the data so the median can be found.
- `std::accumulate` adds the values to compute the mean.
- The `for` loop sums squared deviations for the variance.
- Invalid empty data, NaN and infinity cause `std::invalid_argument`.

This introductory implementation is for small, finite datasets; it does not guarantee overflow-safe accumulation for extremely large values.

## Exercises

1. Work out the mean and median of 1, 1, 5, 9.
2. Implement sample variance with n - 1 and reject samples with fewer than two values.
3. Add an even-length median test.

Further reading: [std::accumulate](https://en.cppreference.com/w/cpp/algorithm/accumulate) and [std::sort](https://en.cppreference.com/w/cpp/algorithm/sort).

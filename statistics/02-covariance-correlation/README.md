# C++ Statistics Lesson 02: Covariance and Pearson Correlation

## Learning goals
Calculate how two numeric samples vary together, and distinguish covariance from correlation.

For paired observations `(x[i], y[i])`, sample covariance is:

`cov(x,y) = sum((x[i]-mean(x)) * (y[i]-mean(y))) / (n-1)`

Pearson correlation divides covariance by the product of sample standard deviations:

`r = cov(x,y) / sqrt(var(x) * var(y))`

Correlation ranges from -1 to 1 for valid nonconstant samples. It is unitless; covariance depends on the measurement units.

## Worked example
Use `x = [1,2,3,4]` and `y = [2,4,6,8]`. Every `y` is twice `x`, so correlation is exactly 1. The sample covariance is 3.333... .

## Build and test

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o correlation
./correlation
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o correlation_tests
./correlation_tests
```

## Exercises
1. Reverse the order of `y` and predict the sign of the correlation.
2. Add 100 to every value in `x`. Does covariance change?
3. Multiply every `y` by 10. Which statistic changes, and why?

## Edge cases
The implementation rejects unequal sample lengths, fewer than two observations, and constant samples for correlation. This introductory implementation assumes finite input values.

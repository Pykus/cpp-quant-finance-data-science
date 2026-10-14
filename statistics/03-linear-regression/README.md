# Simple Linear Regression in C++17

Learn how to fit a straight line to paired measurements using ordinary least squares.

## From a question to a model

Suppose you observe hours studied (`x`) and points scored (`y`). A straight-line model is `predicted_y = intercept + slope * x`. The **slope** describes the predicted change in `y` for one extra unit of `x`. The **intercept** is the predicted value when `x=0`.

A **residual** is `observed_y - predicted_y`. Ordinary least squares chooses the line that minimizes the sum of squared residuals. For paired observations, the solution is:

`slope = sum((x[i]-mean_x)*(y[i]-mean_y)) / sum((x[i]-mean_x)^2)`

`intercept = mean_y - slope*mean_x`

## Numerical example

For `x=[1,2,3,4]` and `y=[3,5,7,9]`, the means are `2.5` and `6`. The numerator is `10`; the denominator is `5`. Thus slope is `2`, intercept is `6-2*2.5=1`, and the line is `y=1+2*x`. At `x=5`, the prediction is `11`.

## Build and test

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o regression
./regression
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o regression_tests
./regression_tests
```

## Exercises

1. What happens to the slope if all `y` values increase by 10? Answer: slope stays 2; intercept increases by 10.
2. Why reject identical `x` values? Answer: the denominator becomes zero.
3. Predict `y` at `x=0` in the example. Answer: 1.

## Interpretation

A fitted line summarizes association, not causation. Extrapolating far beyond observed `x` values can be unreliable. This introductory implementation assumes finite data and does not calculate confidence intervals.

# Bond Price and Macaulay Duration (C++17)

## Goal
Understand how fixed bond cash flows are discounted and how their timing determines interest-rate sensitivity.

A bond with face value `F`, annual coupon rate `c`, yield `y`, and `n` years pays `F*c` each year and repays `F` in the final year.

The price is the sum of discounted cash flows:

`P = sum(CF[t] / (1+y)^t)`

Macaulay duration is the present-value-weighted average payment time:

`D = sum(t * CF[t] / (1+y)^t) / P`

## Run

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o bond
./bond
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o bond_tests
./bond_tests
```

The example prices a three-year bond with face value 1000, a 5% annual coupon, and a 6% yield. Because the yield exceeds the coupon rate, its price is below par.

## Exercises
1. Change the yield from 6% to 4%. Explain the price change.
2. Set the coupon to zero. Verify that duration equals maturity.
3. Compare the duration of otherwise identical two-year and ten-year bonds.

## Assumptions
Annual coupon payments, annual compounding, constant yield, and no credit or transaction costs.

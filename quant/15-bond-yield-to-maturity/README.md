# Lesson 15 — Find bond yield to maturity by bisection

A plain-English C++20 exercise: **given a bond's market price, recover the
annual yield that makes its discounted future payments match that price.**

## Contract

The input is an annual fixed-coupon bond with a positive face value,
nonnegative annual coupon rate, and between 1 and 100 years to maturity.
The model assumes annual payments, no defaults, no fees, and a constant
nonnegative yield compounded annually. Rates and yields are decimals:
`0.05` means 5%.

`bond_price` discounts each future annual coupon and the principal.
`yield_to_maturity` uses **bisection** to invert that price function.
Zero/negative prices, invalid inputs, and yields outside the supported
domain are rejected.

## Why bisection?

For the supported bond type, increasing the yield decreases the
present value of positive cash flows. We can therefore:

1. Start at zero yield and reject prices implying a negative yield.
2. Expand an upper yield bound until its calculated price is low enough.
3. Repeatedly bisect the interval, keeping the half containing the
   matching yield.
4. Stop when the calculated price is within an absolute monetary
   tolerance of the market price.

This is easy to verify and does not need the derivative of price
with respect to yield.

## Worked example

Face value **100**, annual coupon **5%**, maturity **3 years**,
market price **100**. A yield of **5%** reproduces that price:
each coupon is discounted using the yield, and the final year also
pays back the face value.

## Complexity and tests

Pricing one bond takes O(years) time and O(1) extra memory. The
yield finder repeatedly prices bonds; runtime depends on its search
bracket and requested accuracy. Tests include hand-calculated cases,
invalid input, monotonicity and **5,000 deterministic synthetic yields**
recovered from known bond prices.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or: `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o bond_tests && ./bond_tests`.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md). **Exercise:** support
semiannual coupons and test the adjusted formula.

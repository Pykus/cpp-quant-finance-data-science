# Beginner's guide — a bond price and its yield

## What is a bond?

In this exercise, buying a bond means paying a price today in exchange
for promised annual coupons and the repayment of a face value later.

For a face value of 100, an annual 5% coupon is **5 per year**.
With three years remaining, you receive 5 in year 1, 5 in year 2,
and 105 in year 3 (the coupon plus 100 principal).

## Why discount future cash?

Money paid later is not generally equivalent to money paid today.
If the annual yield is 5%, today's value of a payment of 5 next year
is `5/(1.05)`. The year-3 payment is discounted by
three successive factors of 1.05. Adding those present values gives
the calculated bond price.

The **coupon rate** controls the annual promised payments. The
**yield** is the discount rate consistent with the current market
price. They are different inputs and need not be equal.

## Read the C++ code

- `AnnualBond` groups face, annual coupon and years.
- `validate` rejects inputs that violate the simplified contract.
- The loop in `bond_price` repeatedly divides a discount factor
  by `1 + yield`; it adds each coupon and then the principal.
- `yield_to_maturity` tries a midpoint yield. If its price is too
  high, the yield must go **up**; if too low, the yield goes down.
- The absolute price tolerance decides how close the calculated price
  must be. A financial application also needs rounding and
  quotation conventions.

## What this code deliberately omits

Day-count conventions, coupon accrual, changing rates, taxes, callable
bonds, defaults, transaction costs and negative yields. It is not a
production fixed-income pricing library.

## Check your understanding

What happens to the value of a fixed future payment when the required
yield increases? Why can two bonds with the same coupon have different
prices? Add a test using a zero-coupon bond to check your explanation.

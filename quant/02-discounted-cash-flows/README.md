# Quant Lesson 02: Discounted Cash Flows and NPV in C++17

**Goal:** Learn why a future payment differs from money received today, calculate present value, and build and test a C++17 calculator. Start with [Lesson 00](../00-option-basics/) and [Lesson 01](../01-monte-carlo-option-pricing/). No finance background is required.

## 1. Define the terms

A **cash flow** is money received (positive) or paid (negative) at a specified time. **Present value (PV)** is a future cash flow expressed in today's money under an assumed **discount rate**. **Net present value (NPV)** adds the PVs of all inflows and outflows. The rate is a model assumption, not a promised return.

This program uses a **continuously compounded** annual rate, as Lesson 01 does. Its discount factor is `exp(-rate * years)`. In C++, `std::exp` from `<cmath>` computes the exponential function.

## 2. Work through an example

At a 5% continuous annual rate, 105 paid in one year has PV:

`105 * exp(-0.05 * 1) = 99.879...`

Do not confuse continuous compounding with annual compounding, which would give `105 / 1.05 = 100`. For an illustrative project paying 1000 now and receiving 400 after each of the next three years:

`NPV = -1000 + 400*exp(-0.05) + 400*exp(-0.10) + 400*exp(-0.15) = 86.709928...`

This positive NPV is conditional on the fictional cash flows and assumed rate; it is **not** evidence of real investment profitability.

## 3. Follow the code

Open [include/discounting.hpp](include/discounting.hpp). `CashFlow` is a `struct` (a group of named fields) containing `double years` and `double amount`. A `double` holds an approximate decimal number.

In [src/discounting.cpp](src/discounting.cpp), `discount_factor` checks the inputs and computes `std::exp(-annual_rate * years)`; `present_value` multiplies that factor by the cash-flow amount. `net_present_value` uses a `for` loop to add each PV, checks the accumulated result, and returns it. Input and overflow checks prevent many invalid numerical results. Read [src/main.cpp](src/main.cpp) for the synthetic example and its optional discount-rate argument.

## 4. Build, run, test

Requires CMake 3.16+ and a C++17 compiler. From this lesson's directory:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/dcf_demo
./build/dcf_demo 0.10
```

For Visual Studio multi-configuration builds, choose the corresponding Debug/Release executable and use `ctest --test-dir build -C Debug`.

The default example prints each payment's PV and `NPV=86.709928` along with the chosen annual rate. The tests in [tests/test_discounting.cpp](tests/test_discounting.cpp) compare formulas with floating-point tolerances, check a zero rate, negative cash flows and rates, empty schedules, deterministic repeatability, and invalid or overflowing inputs. `ctest` also verifies example output and rejects invalid CLI arguments.

## 5. Exercises

1. Explain why a payment due today has discount factor 1. **Answer:** Its elapsed time is zero, so `exp(0) = 1`.
2. Set the annual rate to zero. Predict the NPV before running. **Answer:** `-1000 + 400 + 400 + 400 = 200`.
3. Increase the rate to 10%. Does the NPV increase or decrease? Compare the program's result.
4. Change the third-year cash flow to 500; rebuild. Explain the effect on NPV.
5. Find the check that rejects negative times. Why is that appropriate for this *future payment* model?

## Assumptions and limitations

All payments are fictional, in one unspecified currency. No taxes, inflation, fees, credit risk, or changing rates are modeled. Calculations use a constant, continuously compounded rate. `double` is finite precision, with possible platform-dependent rounding; exceptionally small PVs may underflow to zero. A positive modeled NPV is not investment advice.

**Next:** Return to Lesson 01 and identify where simulated payoffs are discounted after averaging.

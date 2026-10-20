# Beginner's Guide: How sure are we about a conversion percentage?

## 1. Start with a relatable example

A fictional online shop has a signup button. Out of 300 visitors, 36 sign up. The simplest calculation is:

~~~text
signups / visitors = 36 / 300 = 0.12 = 12 percent
~~~

A conversion is a **yes/no** event. Each *trial* is a visitor who could sign up, and each *success* is one signup.

The observed 12% is a fact about these 300 visits. It is **not** proof that exactly 12% of all future visitors will sign up. Another sample of 300 might produce 29 or 44 signups even if nothing about the button changes.

## 2. What question are we answering?

We would like a sensible range of plausible long-run conversion rates. A **confidence interval** summarizes sampling uncertainty under an explicit model.

A familiar 95% Wilson interval is one way to report this range for proportions. It avoids some of the embarrassing behavior of a naive normal interval when there are only a few visitors, or when nobody (or everybody) converts.

For example, 0 successes out of 10 visitors does not establish a true rate of 0%. The Wilson interval for 0/10 starts at 0 but has an upper bound above 0.

## 3. The calculation, with no skipped symbols

Call successes **s**, trial count **n**, and observed proportion **p = s / n**. Let **z ≈ 1.96** for a nominal two-sided 95% interval. You do not need to derive z to use the formula in this lesson.

~~~text
denom  = 1 + z*z/n
center = (p + (z*z)/(2*n)) / denom
margin = z * sqrt((p*(1-p) + (z*z)/(4*n))/n) / denom
interval = [max(0, center-margin), min(1, center+margin)]
~~~

A worked example: for 50 successes in 100 trials, p = 0.5. This produces approximately **[0.404, 0.596]**, or 40.4%–59.6%.

Why are there two bounds? If we repeated the process with another 100 independently sampled visitors, the observed result would probably change. A single number conceals that variability.

## 4. Reading the demonstration

The program uses these entirely fictional counts:

| Campaign | Visitors | Signups | Observed rate |
| --- | ---: | ---: | ---: |
| A | 300 | 36 | 12% |
| B | 300 | 60 | 20% |

The raw difference is **8 percentage points**. This is different from an **8 percent relative improvement** (which would describe a different quantity).

The example prints two Wilson intervals and this descriptive difference. **It does not conclude that campaign B caused a higher conversion rate.** For that, you would need a sound experiment and a method designed to compare the groups.

## 5. Why the C++ details matter

- The input counts use an unsigned integer type so negative visitors cannot be passed.
- The function throws an exception when n = 0 or s > n. Dividing by zero or processing impossible counts would otherwise mislead a report.
- A floating-point double is used for percentages and square roots.
- The returned structure keeps both the original counts and computed bounds.
- The tests check edge cases and numerical reference values, not merely whether the program compiles.

## 6. Try these exercises

1. Calculate the observed signup rate if 15 visitors out of 100 sign up.
2. Change the sample from 50/100 to 500/1000 and run the code. Does the observed percentage change? How does the interval width change?
3. Compute the intervals for 0/10 and 10/10. Why are they not single-point estimates?
4. What goes wrong if 1000 page visits are all from the same ten people, rather than 1000 independent people?
5. Explain in one sentence why the difference between two observed percentages does not alone prove a causal effect.

References: [NIST Engineering Statistics Handbook](https://www.itl.nist.gov/div898/handbook/) and [cppreference: std::clamp](https://en.cppreference.com/w/cpp/algorithm/clamp).

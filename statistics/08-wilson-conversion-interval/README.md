# Lesson 08 — Wilson confidence intervals for signup conversion

## The real business question

A small online business runs two signup campaigns. Campaign A gets **36 signups from 300 visitors (12%)**, while campaign B gets **60 from 300 (20%)**. The marketing manager wants to report the observed rates **and their sampling uncertainty** instead of treating each percentage as an exact population value.

This tested C++20 library computes the **Wilson score interval for a binary proportion**. It is especially helpful for small samples or observed percentages near zero or one. The demonstration also prints the difference in observed rates, clearly labelled as *descriptive only*.

## Run the actual example and tests

From this directory, with a C++20 compiler:

~~~sh
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o wilson_example
./wilson_example
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic tests.cpp -o wilson_tests
./wilson_tests
~~~

There is also an optional CMake/CTest configuration.

## The algorithm

For n independent binary trials and s successes, p = s/n. With z approximately 1.96 for a two-sided nominal 95% confidence interval:

~~~text
denom  = 1 + z^2 / n
center = (p + z^2 / (2*n)) / denom
margin = (z / denom) * sqrt((p*(1-p) + z^2/(4*n)) / n)
lower  = max(0, center - margin)
upper  = min(1, center + margin)
~~~

The API rejects zero trials, successes exceeding trials, and nonpositive/nonfinite z-values. It returns the original counts alongside the interval, making an audit trail possible. Tests include all-success/all-failure samples, known published numerical examples, complementary symmetry, large counts, and interval behavior across different sample sizes.

**Limitations:** The observations should be independent Bernoulli trials. The confidence level is a long-run property of an interval *procedure*, not a literal 95% probability that this one interval contains a fixed unknown parameter. Comparing two Wilson intervals is **not** a hypothesis test; overlapping intervals do not determine whether an A/B difference is significant. Repeated visitors, selection bias, sequential peeking or changing populations need more careful analysis.

Read [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) before opening the source if you are new to probability.

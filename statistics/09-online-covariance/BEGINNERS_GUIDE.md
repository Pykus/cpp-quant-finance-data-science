# Beginner's Guide: What streaming covariance solves

## 1. Imagine the real-world task

A business analyst receives one row each hour from an online shop, containing website visitors and the number of completed orders. The analyst asks: "When visits increase, do orders tend to increase too?"

A spreadsheet can solve the problem for five rows. For tens of millions of readings, recomputing the answer after every new row would waste time and memory. This project stores a compact mathematical summary and updates it in constant time.

These fictional numbers are small to keep arithmetic visible:

| Week | Website visits X (hundreds) | Paid orders Y (tens) |
| --- | ---: | ---: |
| 1 | 1 | 2 |
| 2 | 2 | 4 |
| 3 | 3 | 5 |
| 4 | 4 | 4 |
| 5 | 5 | 5 |

## 2. Start with averages

The mean of X is (1+2+3+4+5)/5 = 3. The mean of Y is (2+4+5+4+5)/5 = 4.

Subtract the mean from every entry in its respective column. The deviations of X are -2, -1, 0, 1, 2. The deviations of Y are -2, 0, 1, 0, 1.

Multiply paired deviations and sum them:

    (-2 * -2) + (-1 * 0) + (0 * 1) + (1 * 0) + (2 * 1) = 6

For **sample covariance**, divide by n - 1 = 4: **1.5**. For population covariance, divide by n = 5: **1.2**.

The centered squared deviations sum to 10 for X, and 6 for Y. Their sample variances are 10/4 = 2.5 and 6/4 = 1.5.

## 3. Understand Pearson correlation

The standard deviation is the square root of a variance. Correlation removes measurement units by dividing covariance by both standard deviations:

    r = 1.5 / sqrt(2.5 * 1.5) = 0.774596669...

The result is between -1 and +1: +1 means exact positive linear association, -1 means exact negative linear association, and 0 means no linear association. **None of these alone proves causation**. A marketing campaign might affect both orders and visits simultaneously.

## 4. Why store centered sums?

Instead of saving all the rows, keep six running pieces of state:

    n      paired observation count
    mx     running mean of X
    my     running mean of Y
    m2x    sum of squared deviations for X
    m2y    sum of squared deviations for Y
    co     sum of paired deviations

For each new (x,y), first calculate how far the pair lies from the *old* means:

    dx = x - mx
    dy = y - my
    n_new = n + 1

Then shift both means toward the new observation:

    mx_new = mx + dx / n_new
    my_new = my + dy / n_new

Finally adjust the centered sums with the **new** means:

    m2x_new = m2x + dx * (x - mx_new)
    m2y_new = m2y + dy * (y - my_new)
    co_new  = co + dx * (y - my_new)

These are paired Welford-style online updates. The code deliberately computes a prospective state in local variables and verifies it is finite before committing the update.

After at least two records:

    sample covariance = co / (n - 1)
    sample variance X = m2x / (n - 1)
    sample variance Y = m2y / (n - 1)
    correlation = co / (sqrt(m2x) * sqrt(m2y))

The shared (n - 1) factors cancel in the last formula.

## 5. What if you process chunks in parallel?

Suppose one worker summarizes January and another February. You can merge the two summaries without replaying every record. Let A and B have counts a and b, means mxA/mxB and myA/myB. Define:

    dx = mxB - mxA
    dy = myB - myA
    factor = a * b / (a + b)

Then add A and B's centered sums plus the correction:

    merged_co = coA + coB + dx * dy * factor
    merged_m2x = m2xA + m2xB + dx * dx * factor
    merged_m2y = m2yA + m2yB + dy * dy * factor

The new mean of X is mxA + dx*b/(a+b), and likewise for Y. The source code evaluates the factor as (a/(a+b))*b to reduce overflow risk in an intermediate multiplication.

## 6. Read the C++ source

Open [main.cpp](main.cpp) to see the five synthetic data pairs. Follow the add() call to [online_covariance.hpp](online_covariance.hpp).

C++20 practices demonstrated:

- std::size_t counts, rather than signed integers for collection sizes.
- [[nodiscard]] result functions discourage accidental ignored values.
- std::optional<double> makes "undefined" explicit, rather than pretending zero correlation is defined.
- const local variables make intermediate calculations easier to audit.
- no heap-allocated list of all observations and no global mutable state.

There is no mutex. Individual summaries can be built in independent threads and merged later; concurrent calls on the **same object** require external synchronization.

## 7. Exercises

1. Change all X values to 3. What does correlation() return and why?
2. Replace Y with [5,4,3,2,1]. Compute the sign of the correlation before running.
3. Feed only one pair; distinguish population covariance from sample covariance.
4. Split the five example pairs into two groups and merge the summaries. Compare to one pass.
5. Add a synthetic third metric (refunds). How would you calculate correlation between orders and refunds without changing the class itself?

**Answers:** (1) std::nullopt because X has zero variance; (2) -1; (3) population is zero, sample is undefined for n = 1; (4) equal within floating-point precision; (5) use a second OnlineCovariance instance fed with a different pair of aligned metrics.

## 8. Further reading

- [NIST Engineering Statistics Handbook](https://www.itl.nist.gov/div898/handbook/)
- [C++ reference for std::optional](https://en.cppreference.com/w/cpp/utility/optional.html)
- [Online algorithms for variance](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance)

This component is an educational example for interpreting business measurements, not an automated business-decision service.

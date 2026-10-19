# Lesson 07: Detect unusually large readings with the median and MAD

**Practical scenario:** A facilities analyst reviews one sensor's daily temperature readings. Most measurements are near 10°C, but one reports 35°C. Is that a real event or a faulty reading deserving investigation?

An arithmetic mean and standard deviation can be pulled toward extreme measurements. A **median** and **median absolute deviation (MAD)** are robust descriptive alternatives.

1. Find the median (middle reading).
2. Compute the absolute distance of every reading from the median.
3. Take the median of those distances: **MAD**.
4. When MAD > 0, compute a modified z-score: `0.67448975 * (value - median) / MAD`.
5. Flag absolute scores greater than **3.5** for human review.

The software returns **row indices and original scores**, never silently edits or deletes data. When MAD = 0, scores are **undefined**; the program explicitly reports this and flags nothing rather than dividing by zero. This limitation is especially important for repeated rounded measurements.

## Compile, run, test

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o robust_demo
./robust_demo
g++ -std=c++20 -O2 -Wall -Wextra -Werror -pedantic tests.cpp -o robust_tests
./robust_tests
```

The input is seven synthetic Celsius readings, with a single suspicious value of **35.0**. The printed report identifies row **7** (1-based output), while the library returns index **6** (0-based).

An alert does not prove fraud, equipment malfunction, or a faulty data source. Inspect the original context. The method works best for roughly comparable observations; trends, seasonality and grouped populations require a more careful model.

See [BEGINNERS_GUIDE.md](BEGINNERS_GUIDE.md) for every calculation, the degenerate MAD=0 case, and exercises.

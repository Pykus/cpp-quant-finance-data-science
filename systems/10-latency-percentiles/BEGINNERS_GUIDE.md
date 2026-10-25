# Beginner's guide — what does p99 latency mean?

## Latency is a duration

A request can take 100 nanoseconds or 10 milliseconds. It is easy
to average durations, but an average hides occasional slow requests.
A p99 value tells you roughly how slow the fastest **99%** of observed
requests are, under the specific percentile convention you select.

Sort ten measured durations:

    1, 2, 3, 4, 5, 6, 7, 8, 9, 1000

Under **nearest rank**, p50 is the 5th sample (5), p99 is the 10th
sample (1000). With only ten observations, tail percentiles are coarse.
Gather enough data before drawing strong conclusions.

## Why nanoseconds?

C++ `std::chrono` expresses time units in the type system. Avoid
passing a bare integer called `time` without defining its unit.

## Read the source

1. `add` verifies a duration is not negative and stores a number of ns.
2. `summarize` copies all samples because sorting would otherwise
   reorder what the caller collected.
3. `std::sort` orders samples from lowest to highest.
4. `std::ceil(n*p)` selects a 1-based rank. Subtract one to use it
   as an index into the vector.
5. `p99` and `p999` are separate measurements, not a fraction
   of the mean.

## Limitations and next steps

Exact storage eventually consumes too much memory; sorting also
takes time. Approximate histograms, batching, thread-local measurements
and careful clock selection are typical later topics. Never benchmark
on an uncalibrated clock and mistake synthetic examples for real hardware.

**Exercise:** add p90 and compare results for a small sample and
a very large sample containing the same one slow outlier.

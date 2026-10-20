# Streaming variance for a real monitoring service

A service receives millions of response-time measurements. Saving every value just to compute a mean and sample standard deviation wastes memory. Directly subtracting two huge squared sums is also numerically unstable when the readings have a large common offset.

**Solution:** Welford's one-pass algorithm updates count, mean, and squared deviation as each sample arrives, using **O(1) memory**. Chan's merge formula combines summaries from different workers without transferring individual measurements. The code accepts only finite observations, detects count overflow, and distinguishes population from sample variance.

## Try the synthetic four-request example

~~~bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror main.cpp -o streaming_demo
./streaming_demo
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror tests.cpp -o streaming_tests
./streaming_tests
~~~

Expected demo:

~~~text
requests=4
mean_ms=13.000
sample_variance_ms2=6.667
sample_stddev_ms=2.582
min_ms=10.000
max_ms=16.000
~~~

An optional CMake project is included:

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

## Where this fits in production

- HTTP response-time telemetry: maintain one summary per instance, then merge for a region.
- Sensor readings: keep counts, average and spread without storing the full sensor history.
- Analytics pipelines: aggregate worker-level summaries into an overall report.

These are descriptive statistics, not prediction or risk guarantees. An empty accumulator has no defined mean; one reading has a defined mean but **no sample variance**, so accessors return optional values. The accumulator is not thread-safe: give each thread its own instance and merge after synchronization.

Read the [step-by-step beginner's guide](BEGINNERS_GUIDE.md) for the arithmetic and exercises.

#include "latency.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
int checks = 0;
void expect(bool condition, const char* label) {
    ++checks;
    if (!condition) { std::cerr << "FAIL " << label << '\n'; std::exit(EXIT_FAILURE); }
}
template<typename E,typename F>
void throws(F&& action) {
    bool caught = false;
    try { action(); } catch (const E&) { caught = true; }
    expect(caught, "expected error");
}
}
int main() {
    using namespace std::chrono_literals;
    using systems10::LatencyRecorder;

    LatencyRecorder empty;
    expect(empty.size() == 0, "empty");
    throws<std::domain_error>([&] { (void)empty.summarize(); });
    throws<std::invalid_argument>([&] { empty.add(-1ns); });
    expect(empty.size() == 0, "negative add rejected");

    LatencyRecorder one;
    one.add(42ns);
    const auto single = one.summarize();
    expect(single.minimum_ns == 42 && single.median_ns == 42 &&
           single.p99_ns == 42 && single.p999_ns == 42 &&
           single.maximum_ns == 42, "singleton values");

    LatencyRecorder feed;
    for (int i = 10000; i >= 1; --i) {
        feed.add(std::chrono::nanoseconds{i});
        expect(feed.size() == static_cast<std::size_t>(10001 - i),
               "sample recorded");
    }
    const auto s = feed.summarize();
    expect(s.observations == 10000, "sample count");
    expect(s.minimum_ns == 1, "minimum");
    expect(s.median_ns == 5000, "p50");
    expect(s.p99_ns == 9900, "p99");
    expect(s.p999_ns == 9990, "p99.9");
    expect(s.maximum_ns == 10000, "maximum");

    LatencyRecorder outliers;
    for (int i = 0; i < 99; ++i) outliers.add(10ns);
    outliers.add(1'000'000ns);
    const auto tail = outliers.summarize();
    expect(tail.median_ns == 10, "median ignores one outlier");
    expect(tail.p99_ns == 10, "p99 nearest rank");
    expect(tail.p999_ns == 1000000, "p99.9 reflects outlier");

    std::cout << "PASS " << checks << " checks\n";
}

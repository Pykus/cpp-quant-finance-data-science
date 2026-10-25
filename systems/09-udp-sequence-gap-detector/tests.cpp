#include "sequence_tracker.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
std::uint64_t checks = 0;
void expect(bool condition, const char* message) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}
}  // namespace

int main() {
    using systems09::EventKind;
    using systems09::SequenceTracker;

    SequenceTracker tracker;
    expect(!tracker.initialized(), "initially empty");

    auto first = tracker.observe(42);
    expect(first.kind == EventKind::First, "first event");
    expect(first.next_expected == 43, "next after first");
    expect(tracker.observe(43).kind == EventKind::InOrder, "in order");

    auto gap = tracker.observe(46);
    expect(gap.kind == EventKind::Gap, "gap classification");
    expect(gap.missing == 2, "two missing messages");
    expect(gap.next_expected == 47, "next after gap");
    expect(tracker.observe(45).kind == EventKind::LateOrDuplicate, "late");
    expect(tracker.next_expected() == 47, "late must not move expected");
    expect(tracker.observe(46).kind == EventKind::LateOrDuplicate, "duplicate");
    expect(tracker.observe(47).kind == EventKind::InOrder, "resume");

    tracker.reset();
    expect(!tracker.initialized(), "reset clears state");
    expect(tracker.observe(std::numeric_limits<std::uint32_t>::max() - 1).kind == EventKind::First, "wrap first packet");
    expect(tracker.observe(std::numeric_limits<std::uint32_t>::max()).kind ==
           EventKind::InOrder, "wrap step one");
    expect(tracker.observe(0).kind == EventKind::InOrder, "wrap step two");
    auto wrap_gap = tracker.observe(2);
    expect(wrap_gap.kind == EventKind::Gap, "gap crossing wrap");
    expect(wrap_gap.missing == 1, "one missing across wrap");
    expect(tracker.observe(1).kind == EventKind::LateOrDuplicate,
           "late event after wrap");

    tracker.reset();
    expect(tracker.observe(0).kind == EventKind::First, "ambiguity test first packet");
    const auto ambiguity = tracker.observe(0x80000001u);
    expect(ambiguity.kind == EventKind::LateOrDuplicate,
           "ambiguous half-range must be rejected");
    expect(tracker.next_expected() == 1, "ambiguity does not mutate expected");

    // A large deterministic feed checks steady-state behavior cheaply.
    tracker.reset();
    expect(tracker.observe(100).kind == EventKind::First, "feed first packet");
    for (std::uint32_t i = 101; i < 200101; ++i) {
        const auto result = tracker.observe(i);
        expect(result.kind == EventKind::InOrder, "contiguous feed");
        expect(result.missing == 0, "no invented gaps");
    }
    expect(tracker.next_expected() == 200101, "next after feed");

    // One intentional gap, then a late arrival.
    const auto one_more = tracker.observe(200105);
    expect(one_more.kind == EventKind::Gap && one_more.missing == 4,
           "4 missing numbers");
    expect(tracker.observe(200104).kind == EventKind::LateOrDuplicate,
           "late packet following gap");
    expect(tracker.next_expected() == 200106, "advanced next expected");

    std::cout << "PASS " << checks << " checks\n";
}

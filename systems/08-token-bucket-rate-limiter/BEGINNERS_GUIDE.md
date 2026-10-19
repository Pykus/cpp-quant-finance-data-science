# Beginners Guide: Why a Token Bucket?

## 1. The real-world analogy

Imagine a doorman who has **3 tickets** available at the beginning of a concert. Each new guest needs one ticket. New tickets arrive at **2 per second**. The doorman can accept three guests immediately, but a fourth guest arriving at the same moment must wait or be refused.

A token bucket is the same idea for a computer: **tickets = tokens**, **guests = events**, **ticket production = refill rate**.

The maximum number of tickets on hand is called the **burst capacity**. If no guests arrive for a long time, the doorman still has at most 3 tickets; production beyond that is discarded. This stops an idle service from saving an unlimited burst budget.

## 2. Two independent limits

A bucket with capacity 3 and rate 2 tokens/second allows:

- Up to **3 immediate events** after an idle period.
- About **2 events per second** over a long period.

These are deliberately different. Capacity controls **bursts**. Rate controls **sustained throughput**.

This is useful when an HTTP API can temporarily handle a burst but should not be overwhelmed by a continuous flood of requests.

## 3. Work through 1,000 milliseconds

At time zero, the bucket contains 3 tokens.

- A, B, C each consume one: 3 → 2 → 1 → 0.
- Event D immediately requests one more: rejected.
- After 250 milliseconds, only **0.5 token** has been earned: still rejected.
- After 500 milliseconds, a whole token has been earned: accept E.
- After 750 milliseconds, another **0.5 token** has accumulated: reject F.
- After 1,000 milliseconds, that fraction plus another half makes 1: accept G.

The bucket stores fractional progress so that repeated calls do not lose it. The code performs this arithmetic with integers to avoid floating-point surprises.

## 4. How the code computes fractional tokens

For a refill rate of 2 tokens per second:

- 1 second = 1,000 milliseconds.
- 250 ms × 2 = 500 *milli-token units*: half of a token.
- 500 ms × 2 = 1,000 milli-token units: one whole token.
- 750 ms × 2 = 1,500 milli-token units: one token plus 500 units left over.

The class stores whole tokens and the remainder, which is always in the range **0–999**. It also remembers the last time accounted for so a caller can invoke it repeatedly without double counting elapsed time.

## 5. Why not just sleep?

Sleeping slows the entire caller and can introduce timing jitter. A token bucket instead answers a quick **yes/no question**: "Is this event within budget right now?" What to do with a rejected event is a separate application decision. A market-data viewer might drop updates; a job scheduler might enqueue work; an API might return an explicit rate-limit response.

Do **not** drop financial orders or emergency alarms merely because this demo rejects events. Those systems require specialized safety and delivery guarantees.

## 6. What C++ features you will practice

The project uses a class with private state, a constructor that validates configuration, uint32_t/uint64_t for bounded integer math, std::chrono::steady_clock for monotonic timestamps, and [[nodiscard]] to encourage checking the admission result.

The public operation is noexcept, because input timestamps and requests are handled as decisions rather than exceptions. Only invalid construction throws.

## 7. Exercises

1. Configure capacity **5**, refill rate **1** and list the decisions for six requests at time zero and two more after 2 seconds.
2. Implement a per-client limit by storing one TokenBucket per client ID. Decide who owns the buckets and how stale entries are removed.
3. Add a counter for rejected events without adding logging inside the hot path.
4. Study why separate threads cannot safely call try_acquire on the same instance without synchronization.
5. Compare fixed windows and token buckets: which algorithm tolerates a burst at a window boundary?

## 8. Why these tests matter

A naive implementation may refill too soon, allow infinite burst accumulation, lose fractional credit, or overflow while multiplying elapsed time by rate. The tests intentionally target these problems. The 100,000-attempt deterministic load simulation also checks that available tokens never exceed capacity, without needing a real network.

# Fixed-Capacity Ring Buffer in C++17

## Purpose

A **ring buffer** (circular queue) stores items in a fixed-size array. It is useful when a program processes a stream of sensor samples, messages or events without allocating memory for every new item.

A queue uses **first in, first out** (FIFO): the oldest item is removed first.

## How it works

The array has `Capacity` slots. `head_` is the index of the oldest item; `count_` is the number of stored items. A new item goes to `(head_ + count_) % Capacity`. The `%` operator calculates the remainder after division, wrapping an index back to zero at the end of the array.

For capacity 3, after inserting 10, 20, 30, the buffer is full. Removing 10 moves `head_` to 1. Inserting 40 writes to slot 0; the next removals are 20, 30, 40.

`push` returns false when full; `pop` returns false when empty. Both operations take constant time, O(1), because they touch a fixed number of array positions.

## Build and test

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o ring
./ring
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o ring_tests
./ring_tests
```

Expected example output:
```
10
20
```

## Exercises

1. Change capacity to 1 and test full/empty behavior.
2. Draw the array positions after the wraparound example.
3. Add a `front` method that reads the oldest item without removing it.

## Design constraints

The implementation is not thread-safe and requires a default-constructible item type. It does not overwrite data when full. No dynamic allocation occurs inside `push` or `pop` for simple value types such as integers; custom types may have their own allocation behavior.

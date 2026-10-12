# C++ Systems Lesson 02: RAII and Safe File Handling

**Goal:** Understand automatic resource cleanup in C++17. Prerequisites: variables, functions and loops.

## 1. What is a resource?

A resource is something a program acquires and must release: a file handle, memory, a socket or a lock. Forgetting to release it can cause failures.

## 2. What does RAII mean?

**Resource Acquisition Is Initialization (RAII)** is a C++ technique: an object owns a resource and releases it when destroyed. `std::ifstream` owns a file. When its object leaves scope, its destructor closes the file automatically, even when an exception occurs.

## 3. Worked example

Create `example.txt` containing two lines: `alpha` and `beta`. The program reads lines with `std::getline` and prints `lines=2`. Missing files cause an error.

## 4. Build and test

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic main.cpp -o line_count
printf 'alpha\nbeta\n' > example.txt
./line_count example.txt
g++ -std=c++17 -Wall -Wextra -Wpedantic tests.cpp -o raii_tests
./raii_tests
```

Expected output: `lines=2`. Tests exit successfully without printing anything.

## 5. Read the code

- `std::ifstream file(path)` opens a file and owns its handle.
- `if (!file)` detects a failed open.
- `std::getline` reads one line at a time.
- `file.bad()` checks for low-level read errors.
- No manual `close()` is required: destruction cleans up.

## Exercises

1. Count only nonempty lines.
2. Test a file without a final newline.
3. Look up `std::lock_guard`: why does it use the same RAII principle?

Further reading: [std::ifstream](https://en.cppreference.com/w/cpp/io/basic_ifstream), [RAII](https://en.cppreference.com/w/cpp/language/raii).

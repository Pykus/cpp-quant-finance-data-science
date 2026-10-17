#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace systems {

// A bounded, thread-safe FIFO for multiple producers and multiple consumers.
// Blocking operations unblock on close(). Existing items can still be drained.
// This is a mutex-based queue, not a lock-free or real-time data structure.
template <typename T>
class BoundedQueue final {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("queue capacity must be positive");
        }
    }

    BoundedQueue(const BoundedQueue&) = delete;
    BoundedQueue& operator=(const BoundedQueue&) = delete;

    // Blocks when full. Returns false if closed before insertion.
    [[nodiscard]] bool push(T value) {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [this] {
            return closed_ || values_.size() < capacity_;
        });
        if (closed_) {
            return false;
        }
        values_.push_back(std::move(value));
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    // Returns false immediately when full or closed.
    [[nodiscard]] bool try_push(T value) {
        std::unique_lock lock(mutex_);
        if (closed_ || values_.size() == capacity_) {
            return false;
        }
        values_.push_back(std::move(value));
        lock.unlock();
        not_empty_.notify_one();
        return true;
    }

    // Blocks when empty and open. nullopt means closed and fully drained.
    [[nodiscard]] std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [this] {
            return closed_ || !values_.empty();
        });
        if (values_.empty()) {
            return std::nullopt;
        }
        std::optional<T> result{std::move(values_.front())};
        values_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return result;
    }

    [[nodiscard]] std::optional<T> try_pop() {
        std::unique_lock lock(mutex_);
        if (values_.empty()) {
            return std::nullopt;
        }
        std::optional<T> result{std::move(values_.front())};
        values_.pop_front();
        lock.unlock();
        not_full_.notify_one();
        return result;
    }

    // May be called repeatedly; wakes all waiting producers and consumers.
    void close() noexcept {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
        not_full_.notify_all();
    }

    [[nodiscard]] bool is_closed() const {
        std::lock_guard lock(mutex_);
        return closed_;
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard lock(mutex_);
        return values_.size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return capacity_;
    }

private:
    const std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<T> values_;
    bool closed_ = false;
};

}  // namespace systems

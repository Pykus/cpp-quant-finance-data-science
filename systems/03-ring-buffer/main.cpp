#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>

template <typename T, std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity > 0, "capacity must be positive");
    std::array<T, Capacity> data_{};
    std::size_t head_ = 0;
    std::size_t count_ = 0;
public:
    bool push(const T& value) {
        if (count_ == Capacity) return false;
        data_[(head_ + count_) % Capacity] = value;
        ++count_;
        return true;
    }
    bool pop(T& value) {
        if (count_ == 0) return false;
        value = data_[head_];
        head_ = (head_ + 1) % Capacity;
        --count_;
        return true;
    }
    std::size_t size() const { return count_; }
    constexpr std::size_t capacity() const { return Capacity; }
    bool empty() const { return count_ == 0; }
};

#ifndef RING_TEST
int main() {
    RingBuffer<int, 3> queue;
    queue.push(10);
    queue.push(20);
    int value = 0;
    while (queue.pop(value)) std::cout << value << '\n';
}
#endif

#define RING_TEST
#include "main.cpp"
#include <cassert>
int main() {
    RingBuffer<int, 2> q;
    int x = -1;
    assert(q.empty());
    assert(!q.pop(x));
    assert(q.push(1));
    assert(q.push(2));
    assert(!q.push(3));
    assert(q.size() == 2);
    assert(q.pop(x) && x == 1);
    assert(q.push(3));
    assert(q.pop(x) && x == 2);
    assert(q.pop(x) && x == 3);
    assert(q.empty());
}

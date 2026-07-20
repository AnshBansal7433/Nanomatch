#pragma once

#include <atomic>

template<typename T, size_t SIZE>
class SPSCRingBuffer
{
private:

    alignas(64) std::atomic<size_t> head;
    alignas(64) std::atomic<size_t> tail;

    T buffer[SIZE];

public:

    SPSCRingBuffer();

    bool push(const T& item);

    bool pop(T& item);

    bool empty() const;

    bool full() const;
};

template<typename T, size_t SIZE>
SPSCRingBuffer<T, SIZE>::SPSCRingBuffer()
{
    head.store(0);
    tail.store(0);
}
template<typename T, size_t SIZE>
bool SPSCRingBuffer<T, SIZE>::empty() const
{
    return head.load(std::memory_order_acquire) == tail.load(std::memory_order_acquire);
}

template<typename T, size_t SIZE>
bool SPSCRingBuffer<T, SIZE>::full() const
{
    size_t nextTail =
        (tail.load(std::memory_order_relaxed) + 1) % SIZE;

    return nextTail == head.load(std::memory_order_acquire);
}

template<typename T, size_t SIZE>
bool SPSCRingBuffer<T, SIZE>::push(const T& item)
{
    size_t currentTail = tail.load(std::memory_order_relaxed);
    size_t nextTail = (currentTail + 1) % SIZE;

    if (nextTail == head.load(std::memory_order_acquire))
        return false;     // Buffer full

    buffer[currentTail] = item;

    tail.store(nextTail, std::memory_order_release);

    return true;
}

template<typename T, size_t SIZE>
bool SPSCRingBuffer<T, SIZE>::pop(T& item)
{
    size_t currentHead = head.load(std::memory_order_relaxed);

    if (currentHead == tail.load(std::memory_order_acquire))
        return false;      // Buffer empty

    item = buffer[currentHead];

    size_t nextHead = (currentHead + 1) % SIZE;

    head.store(nextHead, std::memory_order_release);

    return true;
}
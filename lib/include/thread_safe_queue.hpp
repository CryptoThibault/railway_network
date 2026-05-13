#pragma once
#include <deque>
#include <mutex>
#include <stdexcept>

template <typename T>
class ThreadSafeQueue
{
public:
    void push_back(const T& newElement);
    void push_front(const T& newElement);
    T pop_back();
    T pop_front();

    bool empty() const;
    size_t size() const;

private:
    std::deque<T> _queue;
    mutable std::mutex _mutex;
};

#include "thread_safe_queue.tpp"
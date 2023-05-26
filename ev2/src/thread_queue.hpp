/**
 * @file thread_queue.hpp
 * @brief 
 * @date 2023-05-22
 * 
 * 
 */
#ifndef EV2_THREAD_QUEUE_HPP
#define EV2_THREAD_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

// see https://stackoverflow.com/questions/15278343/c11-thread-safe-queue
// A threadsafe-queue.
template <class T>
class SafeQueue {
public:
    SafeQueue() : q{}, m{}, c{} {}

    ~SafeQueue() {}

    // Add an element to the queue.
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.emplace(std::move(t));
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    std::optional<T> dequeue(const std::chrono::milliseconds& timeout) {
        std::unique_lock<std::mutex> lock(m);
        // release lock and wait until timeout when duration 
        // see https://en.cppreference.com/w/cpp/thread/condition_variable/wait_for
        if (!c.wait_for(lock, timeout, [&]{ return !q.empty();}))
            return {};
        T val = q.front();
        q.pop();
        return {std::move(val)};
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};

#endif // EV2_THREAD_QUEUE_HPP

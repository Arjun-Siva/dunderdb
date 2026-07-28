#pragma once
#include <condition_variable>
#include <queue>
#include <mutex>

template <typename T>
class CommonQueue {
    public:
    CommonQueue() = default;
    ~CommonQueue() = default;

    // disable copy constructor
    CommonQueue(CommonQueue const &) = delete;
    CommonQueue &operator=(CommonQueue const &) = delete;

    void enqueue(T element) {
        {
            std::scoped_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(element));
        }
        cv_.notify_one();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() -> bool { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // implement an in-place emplace()?

    bool isEmpty() const {
        std::scoped_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    bool size() const {
        std::scoped_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};
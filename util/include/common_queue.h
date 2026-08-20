#pragma once
#include <condition_variable>
#include <queue>
#include <mutex>
#include <optional>

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
            if (closed_) {
                throw std::logic_error("Enqueue called when closed");
            }
            queue_.push(std::move(element));
        }
        cv_.notify_one();
    }

    std::optional<T> dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        // if the queue is not empty or closed_ is true, don't release the lock
        // continue further with the lock, if closed_ is true and then return a null value
        cv_.wait(lock, [&] {
            return !queue_.empty() || closed_;
        });

        if (queue_.empty()) {
            return std::nullopt;
        }

        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    void close()
    {
        {
            std::scoped_lock lock(mutex_);
            closed_ = true;
        }
        // this will wake up the thread waiting on cv_ before dequeue-ing, and it will find that the closed_ has changed
        cv_.notify_all();
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
    bool closed_ = false;
};

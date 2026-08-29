#pragma once

#include "common_queue.h"
#include "unvalidated_message.h"

class IngestionReceiver {
    public:
    // no default constructor, copy constructor
    IngestionReceiver() = delete;
    IngestionReceiver(const IngestionReceiver&) = delete;
    IngestionReceiver(IngestionReceiver&&) = delete;
    IngestionReceiver& operator=(const IngestionReceiver&) = delete;
    IngestionReceiver& operator=(IngestionReceiver&&) = delete;

    ~IngestionReceiver() = default;
    explicit IngestionReceiver(CommonQueue<UnvalidatedMessage>& queue) : insertion_queue_(queue) {};
    void run() const;
    void start();
    void stop();
    void join();
private:
    CommonQueue<UnvalidatedMessage>& insertion_queue_;
    std::thread thread_;
    std::atomic<bool> active_{true};
};

#pragma once

#include "common_queue.h"
#include "../../storage/include/unvalidated_message.h"
 
class NetworkReceiver {
    public:
    // no default constructor, copy constructor
    NetworkReceiver() = delete;
    NetworkReceiver(const NetworkReceiver&) = delete;
    NetworkReceiver(NetworkReceiver&&) = delete;
    NetworkReceiver& operator=(const NetworkReceiver&) = delete;
    NetworkReceiver& operator=(NetworkReceiver&&) = delete;

    ~NetworkReceiver() = default;
    explicit NetworkReceiver(CommonQueue<UnvalidatedMessage>& queue) : insertion_queue_(queue) {};
    void run() const;
    void start();
    void stop();
    void join();
private:
    CommonQueue<UnvalidatedMessage>& insertion_queue_;
    std::thread thread_;
    std::atomic<bool> active_{true};
};

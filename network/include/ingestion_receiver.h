#pragma once

#include "common_queue.h"
#include "ddl_handler.h"
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
    explicit IngestionReceiver(CommonQueue<UnvalidatedMessage>& queue, DDLHandler& ddl_handler) : insertion_queue_(queue), ddl_handler_(ddl_handler) {};
    void run() const;
    void start();
    void stop();
    void join();
private:
    CommonQueue<UnvalidatedMessage>& insertion_queue_;
    DDLHandler& ddl_handler_;
    std::thread thread_;
    std::atomic<bool> active_{true};

    static std::string form_response(const std::string& status, const std::string& msg);
};

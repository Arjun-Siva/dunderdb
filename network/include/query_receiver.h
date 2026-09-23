//
// Created by Arjun on 28/08/2026.
//

#ifndef DUNDERDB_QUERY_RECEIVER_H
#define DUNDERDB_QUERY_RECEIVER_H
#include <thread>

#include "select_handler.h"

class QueryReceiver {
    public:
    QueryReceiver() = delete;
    QueryReceiver(const QueryReceiver&) = delete;
    QueryReceiver(QueryReceiver&&) = delete;
    QueryReceiver& operator=(const QueryReceiver&) = delete;
    QueryReceiver& operator=(QueryReceiver&&) = delete;
    explicit QueryReceiver(SelectHandler& select_handler);

    void run() const;
    void start();
    void stop();
    void join();
private:
    std::thread thread_;
    std::atomic<bool> active_{true};
    SelectHandler& select_handler_;
};

#endif //DUNDERDB_QUERY_RECEIVER_H

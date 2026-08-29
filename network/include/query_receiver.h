//
// Created by Arjun on 28/08/2026.
//

#ifndef DUNDERDB_QUERY_RECEIVER_H
#define DUNDERDB_QUERY_RECEIVER_H
#include <thread>

class QueryReceiver {
    public:
    QueryReceiver() = delete;
    QueryReceiver(const QueryReceiver&) = delete;
    QueryReceiver(QueryReceiver&&) = delete;
    QueryReceiver& operator=(const QueryReceiver&) = delete;
    QueryReceiver& operator=(QueryReceiver&&) = delete;

    ~QueryReceiver() = default;
    explicit QueryReceiver() {};
    void run() const;
    void start();
    void stop();
    void join();
private:
    std::thread thread_;
    std::atomic<bool> active_{true};
};

#endif //DUNDERDB_QUERY_RECEIVER_H

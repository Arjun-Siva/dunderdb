//
// Created by Arjun on 17/06/2026.
//

#ifndef DUNDERDB_VALIDATOR_H
#define DUNDERDB_VALIDATOR_H
#include <unordered_map>
#include "schema.h"
#include "common_queue.h"
#include "unvalidated_message.h"
#include "buffer_map.h"

class Validator {
public:
    Validator() = delete;
    Validator(const Validator&) = delete;
    Validator(Validator&&) = delete;
    Validator& operator=(const Validator&) = delete;
    Validator& operator=(Validator&&) = delete;
    explicit Validator(CommonQueue<UnvalidatedMessage>& queue, BufferMap& buffer_map, CommonQueue<FlushJob>& disk_queue) : insertion_queue_(queue),
        buffer_map_(buffer_map), disk_queue_(disk_queue) {
    };
    ~Validator() = default;

    void add_schema(const Schema& schema);
    void start();
    void join();
private:
    CommonQueue<UnvalidatedMessage>& insertion_queue_;
    std::unordered_map<std::string, Schema> service_schema_map_;
    BufferMap& buffer_map_;
    CommonQueue<FlushJob>& disk_queue_;

    std::thread thread_;

    [[noreturn]] void run();
};
#endif //DUNDERDB_VALIDATOR_H

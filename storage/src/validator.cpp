//
// Created by Arjun on 25/06/2026.
//

#include <iostream>
#include "validator.h"
#include "validated_message.h"

void print_flush_job(const FlushJob& job)
{
    std::cout << "FlushJob {\n";

    std::cout << "  type: ";
    switch (job.type)
    {
        case NEW:
            std::cout << "NEW";
            break;
        case APPEND:
            std::cout << "APPEND";
            break;
        case SEAL:
            std::cout << "SEAL";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }

    std::cout << "\n";
    std::cout << "  service_name: " << job.service_name << "\n";
    std::cout << "  file_name: " << job.file_name << "\n";
    std::cout << "  message_count: " << job.validated_messages.size() << "\n";
    std::cout << "}\n";
}

size_t estimate_size(const RecordsVector& records) {
    size_t estimated_size = sizeof(int64_t); // 8 bytes for timestamp is fixed
    // each individual record could either hold a value or a nullopt

    for (const auto& record : records) {
        if (!record.has_value()) continue;

        // record.value() is a std::variant type value
        if (std::holds_alternative<double>(record.value())) {
            estimated_size += sizeof(double);
        } else {
            estimated_size += std::get<std::string>(record.value()).size();
        }
    }

    return estimated_size;
}

void Validator::add_schema(const Schema& schema) {
    const auto service= schema.get_service_name();
    this->service_schema_map_.emplace(service, schema);
}

void Validator::run() {
    // pop a message from queue

    while (true) {
        auto [service, payload, timestamp_ms] = this->insertion_queue_.dequeue();
        Schema& schema_of_service = this->service_schema_map_.at(service);

        if (std::optional<RecordsVector> validated_payload = schema_of_service.parse_json(payload); validated_payload.has_value()) {
            RecordsVector records = std::move(validated_payload).value();
            const size_t estimated_size = estimate_size(records);

            ValidatedMessage message{std::move(records), timestamp_ms, estimated_size};

            // get buffer map
            ServiceBuffer& serv_buffer = this->buffer_map_.get_buffer(service);
            //if service buffer returns a FlushJob object, push it to disk queue
            std::optional<FlushJob> flush_job = serv_buffer.push_and_get_flush_job(message);

            if (flush_job.has_value()) {
                // print_flush_job(flush_job.value());
                this->disk_queue_.enqueue(std::move(flush_job.value()));
            }

        }
        else {
            // dropped
            // may be a dead letter queue in future
        }
    }
}

void Validator::start() {
    this->thread_ = std::thread(&Validator::run, this);
}

void Validator::join() {
    if (this->thread_.joinable()) this->thread_.join();
}
//
// Created by Arjun on 25/06/2026.
//

#include "validator.h"
#include "validated_message.h"

void print_map(const std::unordered_map<std::string, std::string>& map)
{
    for (const auto& [key, value] : map) {
        std::cout << key << ": " << value << '\n';
    }
}

void Validator::add_schema(const Schema& schema) {
    const auto service= schema.get_service_name();
    this->service_schema_map_.emplace(service, schema);
    // link service disk buffer
}

void Validator::run() {
    // pop a message from queue

    std::cout << "Validator started" << std::endl;

    while (true) {
        auto unvalidated_message = this->insertion_queue_.dequeue();
        Schema& schema_of_service = this->service_schema_map_.at(unvalidated_message.service);
        std::optional<Record> validated_payload = schema_of_service.parse_json(unvalidated_message.payload);

        if (validated_payload.has_value()) {
            print_map(validated_payload.value());
            // ValidatedMessage validMessage{validated_payload.value(), unvalidated_message.timestamp_ms};
        }
        else {
            std::cout<<"discard\n";
        }
    }
}

void Validator::start() {
    this->thread_ = std::thread(&Validator::run, this);
}

void Validator::join() {
    if (this->thread_.joinable()) this->thread_.join();
}
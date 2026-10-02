//
// Created by Arjun on 25/06/2026.
//

#include <iostream>
#include "validator.h"

#include "schema_generator.h"
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
            std::cout << "OTHER";
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
    this->service_schema_map_.insert_or_assign(service, schema);
    this->buffer_manager_.add_buffer(schema);
}

void Validator::erase_schema(const std::string& schema_name) {
    this->service_schema_map_.erase(schema_name);
    this->buffer_manager_.erase_buffer(schema_name);
}

void Validator::run() {
    // pop a message from queue
    // when validator receives a nullopt, it is because the queue is closed and no more messages are available
    while (auto unvalidated_msg = this->insertion_queue_.dequeue()) {
        auto [type, service, payload, timestamp_ms] = unvalidated_msg.value();

        if (type == UnvalidatedMessageType::INSERT) {
            Schema& schema_of_service = this->service_schema_map_.at(service);

            if (std::optional<RecordsVector> validated_payload = schema_of_service.parse_json(payload); validated_payload.has_value()) {
                RecordsVector records = std::move(validated_payload).value();
                const size_t estimated_size = estimate_size(records);

                ValidatedMessage message{std::move(records), timestamp_ms, estimated_size};

                // push message to buffer
                this->buffer_manager_.push_message_to_buffer(service, message);
            }
            else {
                // dropped
                // may be a dead letter queue in future
            }
        }

        else if (type == UnvalidatedMessageType::SCHEMA_NEW) {
            // add the schema to validator's schema map, and create a new service buffer for the new service
            SchemaGenerator schema_gen{payload};
            Schema schema = schema_gen.get_parsed_schema_object();

            this->add_schema(schema);
            this->buffer_manager_.add_buffer(schema);
        }

        else if (type == UnvalidatedMessageType::SCHEMA_UPDATE) {
            SchemaGenerator schema_gen{payload};
            Schema schema = schema_gen.get_parsed_schema_object();

            // force flush existing schema
            this->buffer_manager_.flush_buffer_and_seal(service);

            // add_schema & add_buffer simply replaces the new ServiceBuffer for the existing service name key
            this->add_schema(schema);
            this->buffer_manager_.add_buffer(schema);
        }

        else if (type == UnvalidatedMessageType::SCHEMA_DROP) {
            this->erase_schema(service);
            this->buffer_manager_.erase_buffer(service);
            this->buffer_manager_.delete_service(service);
        }

    }
}

void Validator::start() {
    this->thread_ = std::thread(&Validator::run, this);
}

void Validator::join() {
    if (this->thread_.joinable())
        this->thread_.join();
    // by this point, the insertion queue is empty, but the service buffers are not empty
    // force flush buffer
    this->buffer_manager_.force_flush_all();
}
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
    this->service_schema_map_.insert_or_assign(service, schema);
}

void Validator::erase_schema(const std::string& schema_name) {
    this->service_schema_map_.erase(schema_name);
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

        else if (type == UnvalidatedMessageType::SCHEMA_NEW) {
            // add the schema to validator's schema map, and create a new service buffer for the new service
            SchemaGenerator schema_gen{payload};
            Schema schema = schema_gen.get_parsed_schema_object();

            this->add_schema(schema);
            this->buffer_map_.add_buffer(schema);
        }

        else if (type == UnvalidatedMessageType::SCHEMA_UPDATE) {
            SchemaGenerator schema_gen{payload};
            Schema schema = schema_gen.get_parsed_schema_object();

            // force flush existing schema
            ServiceBuffer& serv_buffer = this->buffer_map_.get_buffer(service);
            std::optional<FlushJob> flush_job = serv_buffer.force_flush_job();
            if (flush_job.has_value()) {
                this->disk_queue_.enqueue(std::move(flush_job.value()));
            }

            // add_schema & add_buffer simply replaces the new ServiceBuffer for the existing service name key
            this->add_schema(schema);
            this->buffer_map_.add_buffer(schema);
        }

        else if (type == UnvalidatedMessageType::SCHEMA_DROP) {
            this->erase_schema(service);
            this->buffer_map_.erase_buffer(service);

            FlushJob drop_service_job{JobType::DROP_SERVICE, service};
            this->disk_queue_.enqueue(std::move(drop_service_job));
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
    std::vector<FlushJob> flush_jobs = this->buffer_map_.force_flush_all();
    for (auto& flush_job : flush_jobs) {
        this->disk_queue_.enqueue(std::move(flush_job));
    }
}
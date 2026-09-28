//
// Created by Arjun on 26/06/2026.
//

#ifndef DUNDERDB_SERVICE_BUFFER_H
#define DUNDERDB_SERVICE_BUFFER_H

#include <string>
#include <vector>
#include <optional>

#include "validated_message.h"
#include "flush_job.h"
#include "schema_serializer.h"

class ServiceBuffer {
public:
    explicit ServiceBuffer(const Schema& schema, const size_t batch_size_kb,
                           const size_t segment_size_mb) : current_batch_size_bytes_(0),
                                                           current_segment_size_bytes_(0),
                                                           current_segment_starting_ts_(0),
                                                           current_segment_ending_ts_(0),
                                                           current_segment_message_count_(0) {
        service_name_ = schema.get_service_name();
        // serialized_schema_bytes_ doesn't include the size of the binary blob itself
        serialized_schema_bytes_ = SchemaSerializer::serialize_schema(schema);
        threshold_batch_size_bytes_ = batch_size_kb * 1024;
        threshold_segment_size_bytes_ = segment_size_mb * 1024 * 1024;
    };

    std::optional<FlushJob> push_and_get_flush_job(ValidatedMessage &message);

    std::optional<FlushJob> force_flush_job();

    // void update_schema(const Schema& new_schema);
    // void update_schema_bytes(const Schema& schema);

    [[nodiscard]] std::string get_service_name() const;
    [[nodiscard]] std::string get_current_segment_name() const;
    [[nodiscard]] size_t get_current_batch_size_bytes() const;
    [[nodiscard]] size_t get_current_segment_size_bytes() const;

private:
    std::string service_name_;
    std::vector<ValidatedMessage> message_buffer_;
    size_t threshold_batch_size_bytes_;
    size_t threshold_segment_size_bytes_;
    std::string current_segment_name_;
    size_t current_batch_size_bytes_;
    size_t current_segment_size_bytes_;
    int64_t current_segment_starting_ts_;
    int64_t current_segment_ending_ts_;
    uint32_t current_segment_message_count_;
    std::vector<std::byte> serialized_schema_bytes_;

    void update_buffer(ValidatedMessage &message);

    FlushJob create_flush_job(JobType type);

    std::string create_segment_file_name() const;
};

#endif //DUNDERDB_SERVICE_BUFFER_H

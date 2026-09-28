//
// Created by Arjun on 26/07/2026.
//
#include "service_buffer.h"

#include <chrono>

void ServiceBuffer::update_buffer(ValidatedMessage &message) {
    if (this->current_segment_size_bytes_ == 0 && this->current_batch_size_bytes_ == 0) {
        // this is the first message of the new file
        this->current_segment_starting_ts_ = message.timestamp;
    }
    this->current_batch_size_bytes_ += message.estimated_size;
    this->current_segment_message_count_ += 1;
    this->message_buffer_.push_back(std::move(message));
}

std::string ServiceBuffer::create_segment_file_name() const {
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    std::string filename = "seg_" + this->service_name_ + "_" + std::to_string(now);
    return filename;
}

FlushJob ServiceBuffer::create_flush_job(const JobType type) {
    if (type == JobType::APPEND) {
        auto append_job = FlushJob{
            type, this->service_name_, std::move(this->message_buffer_), this->current_segment_name_
        };
        this->message_buffer_.clear();
        this->current_segment_size_bytes_ += this->current_batch_size_bytes_;
        this->current_batch_size_bytes_ = 0;
        return append_job;
    }
    if (type == JobType::SEAL) {
        auto seal_job = FlushJob{
            type, this->service_name_, std::move(this->message_buffer_),
            this->current_segment_name_, this->serialized_schema_bytes_,
            this->current_segment_starting_ts_, this->current_segment_ending_ts_, this->current_segment_message_count_
        };
        this->message_buffer_.clear();
        this->current_segment_size_bytes_ = 0;
        this->current_batch_size_bytes_ = 0;
        this->current_segment_name_ = "";
        this->current_segment_message_count_ = 0;
        return seal_job;
    }

    // NEW Flush job
    // create new name for the new segment file
    const std::string new_file_name = this->create_segment_file_name();
    this->current_segment_name_ = new_file_name;

    if (type == JobType::NEW_SEAL) {
        // create a new file and seal it immediately
        auto new_seal_job = FlushJob{
            type, this->service_name_, std::move(this->message_buffer_),
            this->current_segment_name_, this->serialized_schema_bytes_,
            this->current_segment_starting_ts_, this->current_segment_ending_ts_, this->current_segment_message_count_
        };
        this->message_buffer_.clear();
        this->current_segment_size_bytes_ = 0;
        this->current_batch_size_bytes_ = 0;
        this->current_segment_name_ = "";
        this->current_segment_message_count_ = 0;
        return new_seal_job;
    }

    // add the message buffer data
    auto new_job = FlushJob{type, this->service_name_, std::move(this->message_buffer_),
        this->current_segment_name_,this->serialized_schema_bytes_};
    this->message_buffer_.clear();
    this->current_segment_size_bytes_ += this->current_batch_size_bytes_;
    this->current_batch_size_bytes_ = 0;
    return new_job;
}

std::optional<FlushJob> ServiceBuffer::push_and_get_flush_job(ValidatedMessage &message) {
    // add message to the buffer, update batch current size, count, starting ts
    update_buffer(message);

    // if segment file size is zero, create a NEW type FlushJob
    if (this->current_segment_size_bytes_ == 0) {
        if (this->current_batch_size_bytes_ > this->threshold_batch_size_bytes_) {
            return create_flush_job(JobType::NEW);
        }

        return std::nullopt;
    }
    // if batch size exceeds threshold -
    //      decide type of FlushJob - APPEND, SEAL
    //      create new FlushJob
    if (this->current_batch_size_bytes_ > this->threshold_batch_size_bytes_) {
        if (this->current_segment_size_bytes_ > this->threshold_segment_size_bytes_) {
            // this is the last message of the current segment
            this->current_segment_ending_ts_ = message.timestamp;
            return create_flush_job(JobType::SEAL);
        }
        return create_flush_job(JobType::APPEND);
    }
    // else don't return a job
    return std::nullopt;
}

std::optional<FlushJob> ServiceBuffer::force_flush_job() {
    if (this->current_segment_size_bytes_ == 0) {
        if (this->current_batch_size_bytes_ > 0) {
            // rare condition when at the moment of force_flush, the buffer has very few messages and no temp file
            // since it's rare, the number of tiny segment files are rare
            // even if the system is not terminated, the buffer can continue normally after force flush
            this->current_segment_ending_ts_ = this->message_buffer_.back().timestamp;
            return create_flush_job(JobType::NEW_SEAL);
        }
        // even rarer
        return std::nullopt;
    }
    // dump all the contents into a SEAL job
    return create_flush_job(JobType::SEAL);
}

// void ServiceBuffer::update_schema(const Schema &new_schema) {
//     std::string new_schema_name = new_schema.get_service_name();
//
//     if (this->service_name_ != new_schema_name) {
//         throw std::runtime_error("Service name does not match new schema");
//     }
//
//     this->serialized_schema_bytes_ = SchemaSerializer::serialize_schema(new_schema);
// }

std::string ServiceBuffer::get_service_name() const {
    return this->service_name_;
}

std::string ServiceBuffer::get_current_segment_name() const {
    return this->current_segment_name_;
}

size_t ServiceBuffer::get_current_batch_size_bytes() const {
    return this->current_batch_size_bytes_;
}

size_t ServiceBuffer::get_current_segment_size_bytes() const {
    return this->current_segment_size_bytes_;
}

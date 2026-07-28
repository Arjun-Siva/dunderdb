//
// Created by Arjun on 26/07/2026.
//
#include "service_buffer.h"

#include <chrono>

void ServiceBuffer::update_buffer(ValidatedMessage& message) {
    this->current_batch_size_bytes_ += message.estimated_size;
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
        auto append_job = FlushJob{type, this->service_name_, std::move(this->message_buffer_), this->current_segment_name_};
        this->message_buffer_.clear();
        this->current_segment_size_bytes_ += this->current_batch_size_bytes_;
        this->current_batch_size_bytes_ = 0;
        return append_job;
    }
    if (type == JobType::SEAL) {
        auto seal_job = FlushJob{type, this->service_name_, std::move(this->message_buffer_), this->current_segment_name_};
        this->message_buffer_.clear();
        this->current_segment_size_bytes_ = 0;
        this->current_batch_size_bytes_ = 0;
        this->current_segment_name_ = "";
        return seal_job;
    }

    // NEW Flush job
    // create new name for the new segment file
    const std::string new_file_name = this->create_segment_file_name();
    this->current_segment_name_ = new_file_name;

    // add the message buffer data
    auto new_job = FlushJob{type, this->service_name_, std::move(this->message_buffer_), this->current_segment_name_};
    this->message_buffer_.clear();
    this->current_segment_size_bytes_ += this->current_batch_size_bytes_;
    this->current_batch_size_bytes_ = 0;
    return new_job;

}

std::optional<FlushJob> ServiceBuffer::push_and_get_flush_job(ValidatedMessage &message) {
    // add message to the buffer, update batch current size
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
            return create_flush_job(JobType::SEAL);
        }
        return create_flush_job(JobType::APPEND);
    }
    // else don't return a job
    return std::nullopt;
}

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



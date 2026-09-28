//
// Created by Arjun on 26/07/2026.
//
#include "buffer_map.h"

#include <ranges>

BufferMap::BufferMap(size_t buffer_batch_size_kb, size_t segment_size_mb) {
    if (buffer_batch_size_kb <= 0 || segment_size_mb <= 0)
        throw std::invalid_argument("Invalid buffer batch or segment size");

    this->buffer_batch_size_kb_ = buffer_batch_size_kb;
    this->segment_size_mb_ = segment_size_mb;
}

void BufferMap::add_buffer(const std::string& service, std::unique_ptr<ServiceBuffer> buffer) {
    // if the service already exists, it is replaced with the new buffer
    buffers_.insert_or_assign(service, std::move(buffer)); // pointer ownership moved to the unordered map
}

void BufferMap::add_buffer(Schema schema) {
    const std::string service_name = schema.get_service_name();
    auto service_buffer_ptr = std::make_unique<ServiceBuffer>(schema, this->buffer_batch_size_kb_, this->segment_size_mb_); // lives in the heap memory
    this->add_buffer(service_name, std::move(service_buffer_ptr));
}

void BufferMap::erase_buffer(const std::string& service) {
    buffers_.erase(service);
}

ServiceBuffer& BufferMap::get_buffer(const std::string &service) const {
    return *buffers_.at(service);
}

bool BufferMap::contains(const std::string &service) const {
    return buffers_.contains(service);
}

std::vector<FlushJob> BufferMap::force_flush_all() {
    std::vector<FlushJob> jobs;

    for (const auto &buffer: buffers_ | std::views::values) {
        if (std::optional<FlushJob> flush_job = buffer->force_flush_job()) {
            jobs.push_back(std::move(flush_job.value()));
        }
    }

    return jobs;
}

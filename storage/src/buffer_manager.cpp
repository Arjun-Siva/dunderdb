//
// Created by Arjun on 01/10/2026.
//
#include "buffer_manager.h"

void BufferManager::add_buffer(Schema const& schema) {
    // create a lock for the new schema
    const std::string service_name = schema.get_service_name();
    this->service_lock_map_.create(service_name);

    // acquire the new lock
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service_name);

    auto service_buffer_ptr = std::make_unique<ServiceBuffer>(schema, this->buffer_batch_size_kb_, this->segment_size_mb_);

    this->service_buffers_.insert_or_assign(service_name, std::move(service_buffer_ptr));
}

void BufferManager::erase_buffer(const std::string& service) {
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service);
    this->service_buffers_.erase(service);
    // intentionally not removing the entry from LockMap
}

bool BufferManager::contains(const std::string& service) {
    try {
        std::shared_lock<std::shared_mutex> lock = this->service_lock_map_.get_shared_lock(service);
        return this->service_buffers_.contains(service);
    }
    catch (const std::out_of_range&) {
        return false;
    }
}

void BufferManager::push_message_to_buffer(const std::string& service, ValidatedMessage& message) {
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service);
    const std::unique_ptr<ServiceBuffer>& buffer = this->service_buffers_.at(service);

    if (std::optional<FlushJob> flush_job = buffer->push_and_get_flush_job(message); flush_job.has_value()) {
        this->disk_queue_.enqueue(std::move(flush_job.value()));
    }
}

void BufferManager::flush_buffer_and_seal(const std::string& service) {
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service);
    if (std::optional<FlushJob> flush_job = this->service_buffers_.at(service)->force_flush_job(); flush_job.has_value()) {
        this->disk_queue_.enqueue(std::move(flush_job.value()));
    }
}

void BufferManager::flush_buffer_without_sealing(const std::string& service) {
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service);
    if (std::optional<FlushJob> flush_job = this->service_buffers_.at(service)->force_flush_append(); flush_job.has_value()) {
        this->disk_queue_.enqueue(std::move(flush_job.value()));
    }
}

void BufferManager::force_flush_all() {
    for (auto& [service_name, service_buffer] : this->service_buffers_) {
        auto lock = this->service_lock_map_.get_exclusive_lock(service_name);
        if (std::optional<FlushJob> flush_job = service_buffer->force_flush_job(); flush_job.has_value()) {
            this->disk_queue_.enqueue(std::move(flush_job.value()));
        }
    }
}

void BufferManager::delete_service(const std::string& service) {
    std::unique_lock<std::shared_mutex> lock = this->service_lock_map_.get_exclusive_lock(service);
    FlushJob drop_service_job{JobType::DROP_SERVICE, service};
    this->disk_queue_.enqueue(std::move(drop_service_job));
}








//
// Created by Arjun on 01/10/2026.
//

#ifndef DUNDERDB_BUFFER_MANAGER_H
#define DUNDERDB_BUFFER_MANAGER_H
#include "common_queue.h"
#include "lock_map.h"
#include "service_buffer.h"
#include "validated_message.h"

class BufferManager
{
    public:
    BufferManager(CommonQueue<FlushJob>& disk_queue, const size_t buffer_batch_size_kb, const size_t segment_size_mb) :
    disk_queue_(disk_queue),buffer_batch_size_kb_(buffer_batch_size_kb),segment_size_mb_(segment_size_mb) {};
    BufferManager(BufferManager&& other) = delete;
    BufferManager(BufferManager& other) = delete;

    void add_buffer(Schema const& schema);
    void erase_buffer(const std::string &service);
    bool contains(const std::string& service);

    void push_message_to_buffer(const std::string& service, ValidatedMessage& message);
    void flush_buffer_and_seal(const std::string& service);
    void flush_buffer_without_sealing(const std::string& service);
    void force_flush_all();
    void delete_service(const std::string& service);

private:
    CommonQueue<FlushJob>& disk_queue_;
    LockMap service_lock_map_;
    std::unordered_map<std::string, std::unique_ptr<ServiceBuffer>> service_buffers_;
    size_t buffer_batch_size_kb_;
    size_t segment_size_mb_;
};
#endif //DUNDERDB_BUFFER_MANAGER_H

// BufferManager currently is accessed by validator and query handler. Validator could add, erase buffers
// Query handler may flush_buffer_without_sealing
// Exclusive lock for a particular service is acquired before pushing or flushing
// LockMap has its own lock and thread-safe for creating and deleting
// On add_buffer, first create a new lock, acquire that lock and then create new buffer
// On erase_buffer, acquire the lock on the key, drop the buffer from map, release the lock, and then remove from LockMap
// There could be a race condition in erase_buffer, where another thread acquires the lock from LockMap before the key is removed from the LockMap
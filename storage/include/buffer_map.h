//
// Created by Arjun on 27/06/2026.
//

#ifndef DUNDERDB_BUFFER_MAP_H
#define DUNDERDB_BUFFER_MAP_H

#include "service_buffer.h"
#include <memory>
#include <unordered_map>

class BufferMap {
public:
    BufferMap(size_t buffer_batch_size_kb, size_t segment_size_mb);
    void add_buffer(const std::string &service, std::unique_ptr<ServiceBuffer> buffer);
    void add_buffer(Schema schema);
    void erase_buffer(const std::string &service);
    ServiceBuffer& get_buffer(const std::string& service) const;
    bool contains(const std::string& service) const;
    std::vector<FlushJob> force_flush_all();

private:
    std::unordered_map<std::string, std::unique_ptr<ServiceBuffer>> buffers_;
    size_t buffer_batch_size_kb_;
    size_t segment_size_mb_;
};

#endif //DUNDERDB_BUFFER_MAP_H

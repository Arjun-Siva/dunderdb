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
    BufferMap() = default;
    void add_buffer(const std::string &service, std::unique_ptr<ServiceBuffer> buffer);
    ServiceBuffer& get_buffer(const std::string& service) const;
    bool contains(const std::string& service) const;

private:
    std::unordered_map<std::string, std::unique_ptr<ServiceBuffer>> buffers_;
};

#endif //DUNDERDB_BUFFER_MAP_H

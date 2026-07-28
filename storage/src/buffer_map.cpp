//
// Created by Arjun on 26/07/2026.
//
#include "buffer_map.h"

void BufferMap::add_buffer(const std::string& service, std::unique_ptr<ServiceBuffer> buffer) {
    buffers_.emplace(service, std::move(buffer));
}

ServiceBuffer& BufferMap::get_buffer(const std::string &service) const {
    return *buffers_.at(service);
}

bool BufferMap::contains(const std::string &service) const {
    return buffers_.contains(service);
}

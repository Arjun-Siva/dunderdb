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

class ServiceBuffer {
public:
    explicit ServiceBuffer(const std::string& service_name, const int buffer_size_bytes) :service_name_(service_name), buffer_size_bytes_(buffer_size_bytes) {};
    std::optional<FlushJob> push_and_get_flush_job(ValidatedMessage& message);
private:
    std::string service_name_;
    std::vector<ValidatedMessage> message_buffer_;
    int buffer_size_bytes_;
};

#endif //DUNDERDB_SERVICE_BUFFER_H

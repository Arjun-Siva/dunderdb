#include "dunderdb/send_request.h"

#include <utility>

namespace dunderdb
{
    SendRequest::SendRequest(std::string service_name)
        : service_name_(std::move(service_name)) {}

    void SendRequest::add_message(Message message) {
        messages_.push_back(std::move(message));
    }

    const std::string& SendRequest::service_name() const {
        return service_name_;
    }

    const std::vector<Message>& SendRequest::messages() const {
        return messages_;
    }
} // namespace dunderdb

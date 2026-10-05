#pragma once

#include "message.h"

#include <string>
#include <vector>

namespace dunderdb {

    class SendRequest {
    public:
        SendRequest(std::string service_name);

        void add_message(Message message);

        const std::string& service_name() const;
        const std::vector<Message>& messages() const;

    private:
        std::string service_name_;
        std::vector<Message> messages_;
    };

} // namespace dunderdb
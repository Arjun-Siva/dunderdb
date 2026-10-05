#pragma once

#include "message.h"

#include <string>
#include <vector>

namespace dunderdb {

    class Reply {
    public:
        Reply(std::string status, std::string message);

        void add_row(Message row);

        const std::string& status() const;
        const std::string& message() const;
        const std::vector<Message>& rows() const;

        bool successful() const;

    private:
        std::string status_;
        std::string message_;
        std::vector<Message> rows_;
    };

} // namespace dunderdb
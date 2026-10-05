#include "dunderdb/reply.h"

namespace dunderdb
{
    Reply::Reply(std::string status, std::string message)
        : status_(std::move(status)),
          message_(std::move(message)) {}

    void Reply::add_row(Message row) {
        rows_.push_back(std::move(row));
    }

    const std::string& Reply::status() const {
        return status_;
    }

    const std::string& Reply::message() const {
        return message_;
    }

    const std::vector<Message>& Reply::rows() const {
        return rows_;
    }

    bool Reply::successful() const {
        return status_ == "ok";
    }
} // namespace dunderdb

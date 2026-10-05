#include "dunderdb/query_request.h"

#include <regex>
#include <stdexcept>
#include <utility>

namespace dunderdb
{
    QueryRequest::QueryRequest(
        std::string service_name,
        std::string start_time,
        std::string end_time
    )
        : service_name_(std::move(service_name)),
          start_time_(std::move(start_time)),
          end_time_(std::move(end_time)) {
        validate_timestamp(start_time_);
        validate_timestamp(end_time_);
    }

    const std::string& QueryRequest::service_name() const {
        return service_name_;
    }

    const std::string& QueryRequest::start_time() const {
        return start_time_;
    }

    const std::string& QueryRequest::end_time() const {
        return end_time_;
    }

    void QueryRequest::validate_timestamp(const std::string& timestamp) {
        static const std::regex iso_timestamp(
            R"(^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?Z$)"
        );

        if (!std::regex_match(timestamp, iso_timestamp)) {
            throw std::invalid_argument(
                "Invalid timestamp. Expected ISO 8601 UTC format: "
                "YYYY-MM-DDTHH:MM:SS[.fraction]Z"
            );
        }
    }
} // namespace dunderdb

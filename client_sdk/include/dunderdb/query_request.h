#pragma once

#include <string>

namespace dunderdb {

    class QueryRequest {
    public:
        QueryRequest(
            std::string service_name,
            std::string start_time,
            std::string end_time
        );

        const std::string& service_name() const;
        const std::string& start_time() const;
        const std::string& end_time() const;

    private:
        static void validate_timestamp(const std::string& timestamp);

        std::string service_name_;
        std::string start_time_;
        std::string end_time_;
    };

} // namespace dunderdb
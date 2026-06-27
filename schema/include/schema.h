//
// Created by Arjun on 17/06/2026.
//

#ifndef DUNDERDB_SCHEMA_H
#define DUNDERDB_SCHEMA_H

#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <rapidjson/document.h>

#include "column.h"

using Record = std::unordered_map<std::string, std::string>;

class Schema {
public:
    Schema() = default;
    explicit Schema(const std::string& service_name): service_name_{service_name} {}
    void add_column(Column column);
    std::optional<Record> parse_json(std::string_view json) const;
    std::string get_service_name() const;
private:
    std::string service_name_;
    std::vector<std::string> column_order_;
    std::unordered_map<std::string, Column> column_map_;
};

#endif //DUNDERDB_SCHEMA_H

//
// Created by Arjun on 17/06/2026.
//

#ifndef DUNDERDB_SCHEMA_H
#define DUNDERDB_SCHEMA_H

#include <utility>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <variant>

#include "column.h"

using Value = std::variant<double, std::string>;
using OptionalValue = std::optional<Value>;

using RecordsVector = std::vector<OptionalValue>;

class Schema {
public:
    Schema() = default;
    explicit Schema(std::string  service_name): service_name_{std::move(service_name)} {}
    void add_column(const Column& column);
    std::optional<RecordsVector> parse_json(std::string_view json) const;
    std::string get_service_name() const;
    std::vector<Column> get_columns_in_order() const;
private:
    std::string service_name_;
    std::vector<std::string> column_order_;
    std::unordered_map<std::string, Column> column_map_;
};

#endif //DUNDERDB_SCHEMA_H

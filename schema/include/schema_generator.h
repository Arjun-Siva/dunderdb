//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_SCHEMA_GENERATOR_H
#define DUNDERDB_SCHEMA_GENERATOR_H

#include "schema.h"
#include "schema_map.h"

class SchemaGenerator
{
public:
    SchemaGenerator() = delete;
    explicit SchemaGenerator(std::string_view ddl_message);
    std::optional<std::string> get_ddl_error_message(const SchemaMap& schema_map);
    [[nodiscard]] std::string get_ddl_message_type() const;
    [[nodiscard]] std::string get_service_name() const;
    [[nodiscard]] Schema get_parsed_schema_object() const;

private:
    std::string service_name_;
    std::string ddl_type_;
    bool valid_schema_;
    std::string error_message_;
    std::vector<Column> columns_;

    void set_error_message_and_validity(std::string message);
};

#endif //DUNDERDB_SCHEMA_GENERATOR_H

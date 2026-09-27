//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_SCHEMA_GENERATOR_H
#define DUNDERDB_SCHEMA_GENERATOR_H

#include "schema.h"
#include "schema_map.h"
#include "unvalidated_message.h"

class SchemaGenerator
{
public:
    SchemaGenerator() = delete;
    explicit SchemaGenerator(std::string_view ddl_message);
    std::optional<std::string> get_ddl_error_message(const SchemaMap& schema_map);
    UnvalidatedMessageType get_unvalidated_message_type() const;
    Schema get_parsed_schema_object() const;

private:
    std::string service_name_;
    bool valid_schema_;
    std::string error_message_;
    std::vector<Column> columns_;
    UnvalidatedMessageType unvalidated_message_type_;

    void set_error_message_and_validity(std::string message);
};

#endif //DUNDERDB_SCHEMA_GENERATOR_H

//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_SCHEMA_MAP_H
#define DUNDERDB_SCHEMA_MAP_H
#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "schema.h"


class SchemaMap
{
public:
    SchemaMap() = default;
    void add_schema(const std::string& schema_name, std::shared_ptr<Schema> schema);
    void delete_schema(const std::string& schema_name);
    [[nodiscard]] std::shared_ptr<Schema> get_schema(const std::string& schema_name) const;
    [[nodiscard]] bool contains(const std::string& schema_name) const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Schema>> schemas_;
};
#endif //DUNDERDB_SCHEMA_MAP_H

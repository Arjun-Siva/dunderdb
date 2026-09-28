//
// Created by Arjun on 26/09/2026.
//


#include "schema_map.h"

#include <mutex>

void SchemaMap::add_schema(const std::string& schema_name, std::shared_ptr<Schema> schema) {
    std::unique_lock lock(this->mutex_);
    this->schemas_.insert_or_assign(schema_name, schema);
}

void SchemaMap::delete_schema(const std::string& schema_name) {
    std::unique_lock lock(this->mutex_);
    this->schemas_.erase(schema_name);
}

std::shared_ptr<Schema> SchemaMap::get_schema(const std::string& schema_name) const {
    std::shared_lock lock(this->mutex_);
    return this->schemas_.at(schema_name);
}

bool SchemaMap::contains(const std::string& schema_name) const {
    std::shared_lock lock(this->mutex_);
    return this->schemas_.contains(schema_name);
}

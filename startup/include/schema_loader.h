//
// Created by Arjun on 11/08/2026.
//

#ifndef DUNDERDB_SCHEMA_LOADER_H
#define DUNDERDB_SCHEMA_LOADER_H

#include <filesystem>

#include "schema.h"

class SchemaLoader {
public:
    static std::vector<Schema> load_schemas(const std::filesystem::path &schema_directory);
};
#endif //DUNDERDB_SCHEMA_LOADER_H

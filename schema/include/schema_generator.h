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
    SchemaGenerator(std::string& ddl_message);
    bool validate_message(SchemaMap& schema_map);
    Schema generate_schema_object();
};

#endif //DUNDERDB_SCHEMA_GENERATOR_H

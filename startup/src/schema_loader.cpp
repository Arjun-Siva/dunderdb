//
// Created by Arjun on 13/08/2026.
//

#include "schema_loader.h"
#include "file_loader.h"
#include "schema_serializer.h"

std::vector<Schema> SchemaLoader::load_schemas(const std::filesystem::path &schema_directory) {
    // fetch binary data from files in the directory
    std::vector<LoadedFile> files = FileLoader::load_directory(schema_directory);
    std::vector<Schema> schemas;

    for (const auto&[filename, bytes] : files) {
        // deserialize the binary blobs
        Schema schema = SchemaSerializer::deserialize_bytes_to_schema(bytes);
        schemas.push_back(std::move(schema));
    }

    return schemas;
}

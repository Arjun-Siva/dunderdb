//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_DDL_HANDLER_H
#define DUNDERDB_DDL_HANDLER_H

#include <filesystem>

#include "common_queue.h"
#include "index_map.h"
#include "schema.h"
#include "schema_map.h"
#include "unvalidated_message.h"

class DDLHandler
{
public:
    explicit DDLHandler(SchemaMap& schema_map,
                        IndexMap& index_map,
                        CommonQueue<UnvalidatedMessage>& ingestion_queue,
                        const std::string& schemas_directory,
                        const std::string& indexes_directory) :
        master_schema_map_(schema_map), master_index_map_(index_map), ingestion_queue_(ingestion_queue),
        schemas_directory_(schemas_directory), indexes_directory_(indexes_directory) {};

    [[nodiscard]] std::string process_ddl_query(const std::string& json_message) const;

private:
    SchemaMap& master_schema_map_;
    IndexMap& master_index_map_;
    CommonQueue<UnvalidatedMessage>& ingestion_queue_;
    std::filesystem::path schemas_directory_;
    std::filesystem::path indexes_directory_;

    void write_schema_to_disk(Schema& schema, const std::string& service_name) const;
    void delete_schema_from_disk(const std::string& service_name) const;
    static std::string form_response(const std::string& status, const std::string& msg);
};
#endif //DUNDERDB_DDL_HANDLER_H

//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_DDL_HANDLER_H
#define DUNDERDB_DDL_HANDLER_H

#include "common_queue.h"
#include "schema.h"
#include "schema_map.h"
#include "unvalidated_message.h"

class DDLHandler
{
public:
    explicit DDLHandler(SchemaMap& schema_map, CommonQueue<UnvalidatedMessage>& ingestion_queue) :
        master_schema_map_(schema_map), ingestion_queue_(ingestion_queue) {};
    std::string process_ddl_query(std::string& json_message);

private:
    SchemaMap& master_schema_map_;
    CommonQueue<UnvalidatedMessage>& ingestion_queue_;
};
#endif //DUNDERDB_DDL_HANDLER_H

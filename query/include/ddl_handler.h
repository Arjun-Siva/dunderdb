//
// Created by Arjun on 26/09/2026.
//

#ifndef DUNDERDB_DDL_HANDLER_H
#define DUNDERDB_DDL_HANDLER_H
#include <unordered_map>

#include "common_queue.h"
#include "schema.h"
#include "unvalidated_message.h"

class DDLHandler
{
    public:
    DDLHandler();
    std::string process_ddl_query(std::string& json_message);

    private:
    std::unordered_map<std::string, Schema>& master_schema_map_;
    CommonQueue<UnvalidatedMessage>& ingestion_queue_;
};
#endif //DUNDERDB_DDL_HANDLER_H

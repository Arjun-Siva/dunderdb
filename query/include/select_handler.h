//
// Created by Arjun on 03/09/2026.
//

#ifndef DUNDERDB_SELECT_HANDLER_H
#define DUNDERDB_SELECT_HANDLER_H

#include <string>

#include "index_map.h"
#include "validated_message.h"

struct ParsedSelectQuery {
    std::string service_name;
    int64_t start_time;
    int64_t end_time;
};


// parse
// index look up
// load, deserialize files
// filter rows from files
// form reply JSON payload
// reply

class SelectHandler {
    public:
    SelectHandler(); // needs index map, schema map, tmp_file_lock_map, services directory
    std::string get_query_result(std::string json_payload);

private:
    IndexMap& index_map;
    Schema

    ParsedSelectQuery parsed_select_query(std::string& json_payload);
    std::vector<ValidatedMessage> load_messages_from_file(std::string& service_name, std::string& file_name);
    std::string validated_messages_to_json(std::string& service_name, std::vector<ValidatedMessage> validated_messages);
};

#endif //DUNDERDB_SELECT_HANDLER_H

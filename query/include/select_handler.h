//
// Created by Arjun on 03/09/2026.
//

#ifndef DUNDERDB_SELECT_HANDLER_H
#define DUNDERDB_SELECT_HANDLER_H

#include <string>

#include "buffer_manager.h"
#include "index_map.h"
#include "lock_map.h"
#include "parsed_select_query.h"

// parse JSON
// index look up
// load, deserialize files to Segment objects
// get locks for reading tmp files
// filter rows from files
// form reply JSON payload
// reply

class SelectHandler {
    public:
    SelectHandler(IndexMap &indexMap, LockMap &lockMap, BufferManager& buffer_manager, std::filesystem::path services_directory);
    [[nodiscard]] std::string get_query_result(const std::string& json_payload) const;

private:
    IndexMap& index_map_;
    LockMap& tmp_file_lock_map_;
    BufferManager& buffer_manager_;
    std::filesystem::path services_directory_;
    // schema map is not needed, as we store schemas along with the files
    static ParsedSelectQuery parse_select_query(const std::string& json_payload);
};

#endif //DUNDERDB_SELECT_HANDLER_H

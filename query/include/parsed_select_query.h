//
// Created by Arjun on 20/09/2026.
//

#ifndef DUNDERDB_PARSED_SELECT_QUERY_H
#define DUNDERDB_PARSED_SELECT_QUERY_H

#include <string>

struct ParsedSelectQuery {
    std::string service_name;
    int64_t start_time;
    int64_t end_time;
};


#endif //DUNDERDB_PARSED_SELECT_QUERY_H

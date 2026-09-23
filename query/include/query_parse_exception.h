//
// Created by Arjun on 20/09/2026.
//

#ifndef DUNDERDB_QUERY_PARSE_EXCEPTION_H
#define DUNDERDB_QUERY_PARSE_EXCEPTION_H
#include <stdexcept>

class QueryParseException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
};
#endif //DUNDERDB_QUERY_PARSE_EXCEPTION_H

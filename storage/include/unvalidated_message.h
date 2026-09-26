#pragma once
#include <string>

enum UnvalidatedMessageType {
    INSERT,
    DELETE,
    SCHEMA_NEW,
    SCHEMA_DROP,
    SCHEMA_UPDATE
};

struct UnvalidatedMessage {
    UnvalidatedMessageType type;
    std::string service;
    std::string payload;
    int64_t timestamp_ms;
};
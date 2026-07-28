#pragma once
#include <string>

struct UnvalidatedMessage {
    std::string service;
    std::string payload;
    int64_t timestamp_ms;
};
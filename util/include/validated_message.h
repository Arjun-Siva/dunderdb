//
// Created by Arjun on 25/06/2026.
//

#ifndef DUNDERDB_VALIDATED_MESSAGE_H
#define DUNDERDB_VALIDATED_MESSAGE_H
#include <cstdint>
#include <unordered_map>


struct ValidatedMessage {
    std::unordered_map<std::string, std::string> record;
    int64_t timestamp;
};

#endif //DUNDERDB_VALIDATED_MESSAGE_H

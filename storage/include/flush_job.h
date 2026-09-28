//
// Created by Arjun on 26/06/2026.
//

#ifndef DUNDERDB_FLUSH_JOB_H
#define DUNDERDB_FLUSH_JOB_H

#include <string>
#include <vector>
#include "validated_message.h"


enum JobType {
    NEW,
    APPEND,
    SEAL,
    NEW_SEAL,
    DROP_SERVICE,
    REMOVE
};

struct FlushJob {
    JobType type;
    std::string service_name;
    std::vector <ValidatedMessage> validated_messages;
    std::string file_name;
    std::vector<std::byte> header_bytes;
    int64_t segment_starting_ts;
    int64_t segment_ending_ts;
    uint32_t segment_message_count;
};

#endif //DUNDERDB_FLUSH_JOB_H

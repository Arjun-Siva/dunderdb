//
// Created by Arjun on 26/06/2026.
//

#ifndef DUNDERDB_FLUSH_JOB_H
#define DUNDERDB_FLUSH_JOB_H

#include <string>
#include <vector>

#include "sequenced_message.h"
#include "segment_header.h"

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
    std::vector <SequencedMessage> sequenced_messages;
    std::optional<SegmentHeader> header;
};

#endif //DUNDERDB_FLUSH_JOB_H

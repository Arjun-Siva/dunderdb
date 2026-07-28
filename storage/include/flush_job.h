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
    SEAL
};

struct FlushJob {
    JobType type;
    std::string service_name;
    std::vector <ValidatedMessage> validate_messages;
    std::string file_name;
};

#endif //DUNDERDB_FLUSH_JOB_H

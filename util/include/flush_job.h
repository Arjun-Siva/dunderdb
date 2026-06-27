//
// Created by Arjun on 26/06/2026.
//

#ifndef DUNDERDB_FLUSH_JOB_H
#define DUNDERDB_FLUSH_JOB_H

#include <string>
#include <vector>
#include "validated_message.h"

struct FlushJob {
    std::string service_name;
    std::vector <ValidatedMessage> validate_messages;
};

#endif //DUNDERDB_FLUSH_JOB_H

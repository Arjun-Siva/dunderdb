//
// Created by Arjun on 25/06/2026.
//

#ifndef DUNDERDB_VALIDATED_MESSAGE_H
#define DUNDERDB_VALIDATED_MESSAGE_H
#include <string>
#include "schema.h"


struct ValidatedMessage {
    RecordsVector records;
    int64_t timestamp;
    size_t estimated_size;
};

#endif //DUNDERDB_VALIDATED_MESSAGE_H

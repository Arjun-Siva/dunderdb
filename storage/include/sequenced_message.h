//
// Created by Arjun on 19/10/2026.
//

#ifndef DUNDERDB_SEQUENCED_MESSAGE_H
#define DUNDERDB_SEQUENCED_MESSAGE_H

#include <cstdint>
#include "validated_message.h"

struct SequencedMessage
{
    uint64_t message_id;
    ValidatedMessage message;
};

#endif //DUNDERDB_SEQUENCED_MESSAGE_H

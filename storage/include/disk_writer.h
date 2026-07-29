//
// Created by Arjun on 29/07/2026.
//

#ifndef DUNDERDB_DISK_WRITER_H
#define DUNDERDB_DISK_WRITER_H

#include "serializer.h"

// pops a flush job from disk queue
// serialize messages
// if NEW, create a new file seg_servicename.tmp, writer header, append the records
// if APPEND, append records
// if SEAL, append records, rename seg_servicename.tmp to seg_servicename_timestamp.ddb, update index
class DiskWriter {
public:
    DiskWriter() = default;

private:
    std::string files_directory;
};

#endif //DUNDERDB_DISK_WRITER_H

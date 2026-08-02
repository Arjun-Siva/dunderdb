//
// Created by Arjun on 01/08/2026.
//

#ifndef DUNDERDB_SEGMENT_METADATA_H
#define DUNDERDB_SEGMENT_METADATA_H
#include <cstdint>
#include <string>

struct SegmentMetadata {
    int64_t start_ts;
    int64_t end_ts;
    uint32_t count;
    std::string filename;
};

#endif //DUNDERDB_SEGMENT_METADATA_H

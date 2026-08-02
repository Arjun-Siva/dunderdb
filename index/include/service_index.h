//
// Created by Arjun on 01/08/2026.
//

#ifndef DUNDERDB_SERVICE_INDEX_H
#define DUNDERDB_SERVICE_INDEX_H

#include <filesystem>
#include <shared_mutex>
#include <utility>
#include <vector>
#include <fstream>

#include "segment_metadata.h"

class ServiceIndex {
public:
    explicit
    ServiceIndex(std::string service_name, const std::string &index_file_directory) : index_file_directory_(
        index_file_directory), service_name_(std::move(service_name)) {
    };
    void append_segment_metadata(SegmentMetadata& new_segment_metadata);
    void load_segment_metadata_list(std::vector<SegmentMetadata>& segment_metadata_list);

    std::vector<std::string> get_file_names(int64_t start_ts, int64_t end_ts);

private:
    mutable std::shared_mutex mutex_;
    std::vector<SegmentMetadata> segments_;
    std::filesystem::path index_file_directory_;
    std::string service_name_;

    void append_metadata_to_disk(SegmentMetadata& new_segment_metadata);
    static std::vector<std::byte> serialize_segment_metadata(const SegmentMetadata& segment_metadata);
};

#endif //DUNDERDB_SERVICE_INDEX_H

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
    explicit ServiceIndex(std::string service_name, const std::string &index_file_directory) : index_file_directory_(
        index_file_directory), service_name_(std::move(service_name)) {
    };
    void load_segment_metadata_list(std::vector<SegmentMetadata>& segment_metadata_list);
    void append_segment_metadata(SegmentMetadata& new_segment_metadata);
    std::vector<std::string> index_lookup_time_range(int64_t start_ts, int64_t end_ts) const;
    // TODO: take care of broken/incomplete files on loading

private:
    mutable std::shared_mutex mutex_;
    std::vector<SegmentMetadata> segments_;
    std::filesystem::path index_file_directory_;
    std::string service_name_;

    void append_metadata_to_disk(SegmentMetadata& new_segment_metadata);
    // size_t find_lower_bound(int64_t q_start_ts) const;
    // size_t find_upper_bound(size_t starting_index, int64_t q_end_ts) const;
};

#endif //DUNDERDB_SERVICE_INDEX_H

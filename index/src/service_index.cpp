//
// Created by Arjun on 01/08/2026.
//
#include <mutex>

#include "service_index.h"
#include "index_serializer.h"

#include <iostream>

void ServiceIndex::append_metadata_to_disk(SegmentMetadata& new_segment_metadata) {
    std::vector<std::byte> serialized_metadata = IndexSerializer::serialize_segment_metadata(new_segment_metadata);

    const std::filesystem::path index_file_path = this->index_file_directory_ / std::string(this->service_name_ + ".idx");

    std::ofstream file(
        index_file_path,
        std::ios::binary | std::ios::app
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + index_file_path.string()
        );
    }

    // Append serialized indexes metadata
    file.write(
        reinterpret_cast<const char*>(serialized_metadata.data()),
        static_cast<std::streamsize>(serialized_metadata.size())
    );

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + index_file_path.string()
        );
    }

    std::cout<<"Index write"<<std::endl;
}

void ServiceIndex::load_segment_metadata_list(std::vector<SegmentMetadata> &segment_metadata_list) {
    this->segments_ = std::move(segment_metadata_list);
}

void ServiceIndex::append_segment_metadata(SegmentMetadata& new_segment_metadata) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    // append to disk first
    append_metadata_to_disk(new_segment_metadata);

    // append in memory
    this->segments_.push_back(new_segment_metadata);
}

size_t find_lower_bound(const std::vector<SegmentMetadata>& segments, const int64_t q_start_ts) {
    size_t left = 0;
    size_t right = segments.size(); // exclusive

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;

        if (segments[mid].end_ts < q_start_ts) {
            // This segment ends before the query starts.
            // Everything up to and including mid is irrelevant.
            left = mid + 1;
        } else {
            // segments[mid].e >= Qs
            // mid could be the answer, so keep it.
            right = mid;
        }
    }

    return left;
}

size_t find_upper_bound(const std::vector<SegmentMetadata>& segments, const size_t starting_index, const int64_t q_end_ts) {
    size_t left = starting_index;
    size_t right = segments.size(); // exclusive

    while (left < right)
    {
        size_t mid = left + (right - left) / 2;

        if (segments[mid].end_ts <= q_end_ts)
        {
            // This segment can overlap the query.
            // Look further right for the last possible one.
            left = mid + 1;
        }
        else
        {
            // s > Qe, so this and everything after it
            // cannot overlap the query.
            right = mid;
        }
    }

    return left - 1;
}

std::vector<std::string> ServiceIndex::index_lookup_time_range(const int64_t start_ts, const int64_t end_ts) const {
    std::vector<std::string> result_file_names;
    // shared lock for simultaneous reads
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const size_t lower_bound = find_lower_bound(this->segments_, start_ts);

    if (lower_bound == this->segments_.size()) { // query starts after all stored data
        return result_file_names;
    }

    const size_t upper_bound = find_upper_bound(this->segments_, lower_bound, end_ts);

    // if lb > ub, in case where both start and end_ts are within a gap, no files will be added to result

    for (size_t i = lower_bound; i <= upper_bound; ++i) {
        result_file_names.push_back(this->segments_[i].filename);
    }

    // add .tmp file if needed
    if (this->segments_.empty() || this->segments_.back().end_ts < end_ts) {
        const std::string tmp_file = "seg_" + this->service_name_ + ".tmp";
        result_file_names.push_back(tmp_file);
    }

    return result_file_names;
}

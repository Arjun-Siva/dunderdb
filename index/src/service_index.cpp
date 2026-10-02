//
// Created by Arjun on 01/08/2026.
//
#include <mutex>

#include "service_index.h"
#include "index_serializer.h"

#include <iostream>
#include <algorithm>

#include "../../util/include/time_converter.h"

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
    this->segments_.push_back(std::move(new_segment_metadata));
}

std::vector<std::string> ServiceIndex::index_lookup_time_range(const int64_t start_ts, const int64_t end_ts) const {
    std::vector<std::string> result_file_names;
    // shared lock for simultaneous reads
    std::shared_lock<std::shared_mutex> lock(mutex_);

    // 1. Find the FIRST segment that doesn't end before our query starts.
    // This is our inclusive lower bound.
    const auto it_start = std::lower_bound(this->segments_.begin(), this->segments_.end(), start_ts,
        [](const SegmentMetadata& seg, const int64_t q_start) {
            return seg.end_ts < q_start; // True if segment is entirely in the past
        });

    // 2. Find the FIRST segment that starts strictly after our query ends.
    // This acts as our exclusive upper bound.
    const auto it_end = std::upper_bound(it_start, this->segments_.end(), end_ts,
        [](const int64_t q_end, const SegmentMetadata& seg) {
            return q_end < seg.start_ts; // True if segment is entirely in the future
        });

    // 3. Iterate through all overlapping segments.
    // If the query fell perfectly in a gap, it_start will equal it_end,
    // and this loop simply won't execute. (No underflows!)
    for (auto it = it_start; it != it_end; ++it) {
        result_file_names.push_back(it->filename);
    }

    // 4. Add .tmp file if needed
    if (this->segments_.empty() || this->segments_.back().end_ts < end_ts) {
        const std::string tmp_file = "seg_" + this->service_name_ + ".tmp";
        result_file_names.push_back(tmp_file);
    }

    return result_file_names;
}

void ServiceIndex::erase_index_file_from_disk() const {
    // self annihilation ooo
    const std::filesystem::path index_file_path = this->index_file_directory_ / std::string(this->service_name_ + ".idx");
    std::filesystem::remove(index_file_path);
}

void ServiceIndex::print_index_file_ranges() const {
    std::cout<<this->service_name_<<":"<<std::endl;
    for (const auto& [start_ts, end_ts, count, filename] : this->segments_) {
        std::cout<<filename<<" : "<<TimeConverter::epoch_ms_to_iso(start_ts)<<" - "<<TimeConverter::epoch_ms_to_iso(end_ts)<<";"<<count<<std::endl;
    }
}

//
// Created by Arjun on 01/08/2026.
//
#include <mutex>

#include "service_index.h"
#include "index_serializer.h"

#include <iostream>

void ServiceIndex::append_metadata_to_disk(SegmentMetadata &new_segment_metadata) {
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

//
// Created by Arjun on 01/08/2026.
//
#include <mutex>

#include "service_index.h"

#include <iostream>

template<typename T>
void append_to_buffer(std::vector<std::byte>& buffer, const T& value) {
    // checked during compile time if T is POD
    static_assert(std::is_trivially_copyable_v<T>);
    // convert all types to array of bytes
    const auto* ptr = reinterpret_cast<const std::byte*>(&value);
    // append the array of bytes to the vector
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
}

std::vector<std::byte> ServiceIndex::serialize_segment_metadata(const SegmentMetadata& segment_metadata) {
    // | start_time | end_time | count | filename length | filename string |

    std::vector<std::byte> buffer;

    // Reserve buffer size
    buffer.reserve((sizeof(int64_t) * 2) + sizeof(uint32_t) + sizeof(uint16_t) + segment_metadata.filename.length());

    append_to_buffer(buffer, segment_metadata.start_ts);
    append_to_buffer(buffer, segment_metadata.end_ts);
    append_to_buffer(buffer, segment_metadata.count);
    append_to_buffer(buffer, static_cast<uint16_t>(segment_metadata.filename.length()));
    append_to_buffer(buffer, segment_metadata.filename.data());

    return buffer;
}

void ServiceIndex::append_metadata_to_disk(SegmentMetadata &new_segment_metadata) {
    std::vector<std::byte> serialized_metadata = serialize_segment_metadata(new_segment_metadata);

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

void ServiceIndex::append_segment_metadata(SegmentMetadata& new_segment_metadata) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    // append to disk first
    append_metadata_to_disk(new_segment_metadata);

    // append in memory
    this->segments_.push_back(new_segment_metadata);
}

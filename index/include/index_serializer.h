//
// Created by Arjun on 05/08/2026.
//

#ifndef DUNDERDB_INDEX_SERIALIZER_H
#define DUNDERDB_INDEX_SERIALIZER_H

#include <cstring>
#include <span>
#include <vector>

#include "segment_metadata.h"

class IndexSerializer {
    public:
    static std::vector<std::byte> serialize_segment_metadata(const SegmentMetadata& segment_metadata) {
        // | object size | start_time | end_time | count | filename length | filename string |

        std::vector<std::byte> buffer;

        const uint32_t object_size =
            sizeof(segment_metadata.start_ts) +
            sizeof(segment_metadata.end_ts) +
            sizeof(segment_metadata.count) +
            sizeof(uint16_t) +               // filename length
            segment_metadata.filename.size();

        // Reserve buffer size
        buffer.reserve(sizeof(object_size) + object_size);

        append_to_buffer(buffer, object_size);
        append_to_buffer(buffer, segment_metadata.start_ts);
        append_to_buffer(buffer, segment_metadata.end_ts);
        append_to_buffer(buffer, segment_metadata.count);
        // filename length will be stored in uint16_t
        append_string_to_buffer(buffer, segment_metadata.filename);

        return buffer;
    }

    static SegmentMetadata deserialize_bytes_of_single_metadata_object(const std::span<const std::byte> bytes) {
        SegmentMetadata metadata;
        size_t offset = 0;

        auto read = [&](auto& value) {
            // dst, src, size
            std::memcpy(&value, bytes.data() + offset, sizeof(value));
            offset += sizeof(value);
        };

        read(metadata.start_ts);
        read(metadata.end_ts);
        read(metadata.count);

        uint16_t filename_length;
        read(filename_length);

        metadata.filename.resize(filename_length);

        std::memcpy(metadata.filename.data(), bytes.data() + offset, filename_length);

        return metadata;
    }

    static std::vector<SegmentMetadata> deserialize_bytes_of_index_file(const std::vector<std::byte>& bytes_vector) {
        std::vector<SegmentMetadata> segments;

        size_t offset = 0;

        while (offset < bytes_vector.size()) {
            uint32_t object_size;

            if (offset + sizeof(uint32_t) > bytes_vector.size())
                throw std::runtime_error("Corrupted index file");

            std::memcpy(&object_size,
                        bytes_vector.data() + offset,
                        sizeof(object_size));

            offset += sizeof(object_size);

            if (offset + object_size > bytes_vector.size())
                throw std::runtime_error("Corrupted index file");

            auto metadata =
                deserialize_bytes_of_single_metadata_object(
                    std::span<const std::byte>(
                        bytes_vector.data() + offset,
                        object_size));

            segments.push_back(std::move(metadata));

            offset += object_size;
        }

        return segments;
    }

private:
    template<typename T>
    static void append_to_buffer(std::vector<std::byte>& buffer, const T& value) {
        // checked during compile time if T is POD
        static_assert(std::is_trivially_copyable_v<T>);
        // convert all types to array of bytes
        const auto* ptr = reinterpret_cast<const std::byte*>(&value);
        // append the array of bytes to the vector
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    static void append_string_to_buffer(std::vector<std::byte>& buffer, const std::string& value)
    {
        // uint16_t can represent ~ 65K
        const uint16_t size = static_cast<uint16_t>(value.size());
        append_to_buffer(buffer, size);
        const auto* ptr = reinterpret_cast<const std::byte*>(value.data());
        buffer.insert(buffer.end(), ptr, ptr + size);
    }
};


#endif //DUNDERDB_INDEX_SERIALIZER_H

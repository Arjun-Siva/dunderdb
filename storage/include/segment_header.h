//
// Created by Arjun on 20/10/2026.
//

#ifndef DUNDERDB_SEGMENT_HEADER_H
#define DUNDERDB_SEGMENT_HEADER_H
#include <cstdint>
#include <string>
#include <vector>
#include <span>

class SegmentHeader
{
public:
    SegmentHeader(std::string filename, const std::vector<std::byte>& schema_bytes) :
        filename_(std::move(filename)),
        schema_bytes_(schema_bytes),
        segment_starting_ts_(0),
        segment_ending_ts_(0),
        segment_message_count_(0),
        max_message_id_(0) {}

    void set_segment_starting_ts(int64_t segment_starting_ts);
    void set_segment_ending_ts(int64_t segment_ending_ts);
    void set_segment_message_count(uint32_t segment_message_count);
    void set_max_message_id(uint64_t max_message_id);

    std::string get_filename() const;
    std::vector<std::byte> get_schema_bytes() const;
    int64_t get_segment_starting_ts() const;
    int64_t get_segment_ending_ts() const;
    uint32_t get_segment_message_count() const;
    uint64_t get_max_message_id() const;

    explicit SegmentHeader(std::span<const std::byte> binary_header);

    std::vector<std::byte> convert_to_bytes() const;

private:
    std::string filename_;
    std::vector<std::byte> schema_bytes_;
    int64_t segment_starting_ts_;
    int64_t segment_ending_ts_;
    uint32_t segment_message_count_;
    uint64_t max_message_id_;

    template <typename T>
    static void append_to_buffer(std::vector<std::byte>& buffer, const T& value) {
        // checked during compile time if T is POD
        static_assert(std::is_trivially_copyable_v<T>);
        // convert all types to array of bytes
        const auto* ptr = reinterpret_cast<const std::byte*>(&value);
        // append the array of bytes to the vector
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    static void append_string_to_buffer(std::vector<std::byte>& buffer, const std::string& value) {
        const uint8_t size = static_cast<uint8_t>(value.size());
        append_to_buffer(buffer, size);
        const auto* ptr = reinterpret_cast<const std::byte*>(value.data());
        buffer.insert(buffer.end(), ptr, ptr + size);
    }
};

#endif //DUNDERDB_SEGMENT_HEADER_H

//
// Created by Arjun on 20/10/2026.
//
#include "segment_header.h"

#include <cstring>
#include <stdexcept>

void SegmentHeader::set_segment_starting_ts(const int64_t segment_starting_ts) {
    this->segment_starting_ts_ = segment_starting_ts;
}

void SegmentHeader::set_segment_ending_ts(const int64_t segment_ending_ts) {
    this->segment_ending_ts_ = segment_ending_ts;
}

void SegmentHeader::set_segment_message_count(const uint32_t segment_message_count) {
    this->segment_message_count_ = segment_message_count;
}

void SegmentHeader::set_max_message_id(const uint64_t max_message_id) {
    this->max_message_id_ = max_message_id;
}

std::string SegmentHeader::get_filename() const {
    return this->filename_;
}

std::vector<std::byte> SegmentHeader::get_schema_bytes() const {
    return this->schema_bytes_;
}

int64_t SegmentHeader::get_segment_starting_ts() const {
    return this->segment_starting_ts_;
}

int64_t SegmentHeader::get_segment_ending_ts() const {
    return this->segment_ending_ts_;
}

uint32_t SegmentHeader::get_segment_message_count() const {
    return this->segment_message_count_;
}

uint64_t SegmentHeader::get_max_message_id() const {
    return this->max_message_id_;
}

SegmentHeader::SegmentHeader(std::span<const std::byte> binary_header) {
    // deserialize bytes
    // NOTE: bytes_header doesn't contain the size of the header
    // filename length(uint8_t) | filename(length) | schema size(uint16_t) | schema(size) | starting_ts(int64_t) | ending_ts(int64_t) | min_message_id(uint64_t) | max_message_id(uint64_t)

    size_t offset = 0;

    if (offset + sizeof(uint8_t) > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    uint8_t filename_length;
    std::memcpy(&filename_length, binary_header.data() + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    if (offset + filename_length > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::string filename(
                    reinterpret_cast<const char*>(binary_header.data() + offset),
                    filename_length
                );

    this->filename_ = std::move(filename);
    offset += filename_length;

    if (offset + sizeof(uint16_t) > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    uint16_t schema_size;
    std::memcpy(&schema_size, binary_header.data() + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    if (offset + schema_size > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::vector<std::byte> schema_bytes(
    binary_header.begin() + offset,
    binary_header.begin() + offset + schema_size
    );

    this->schema_bytes_ = std::move(schema_bytes);
    offset += schema_size;

    int64_t segment_starting_ts;
    int64_t segment_ending_ts;

    if (offset + (sizeof(int64_t) * 2) > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&segment_starting_ts, binary_header.data() + offset, sizeof(int64_t));
    this->segment_starting_ts_ = segment_starting_ts;
    offset += sizeof(int64_t);

    std::memcpy(&segment_ending_ts, binary_header.data() + offset, sizeof(int64_t));
    this->segment_ending_ts_ = segment_ending_ts;
    offset += sizeof(int64_t);

    uint32_t segment_message_count;
    if (offset + sizeof(uint32_t) > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&segment_message_count, binary_header.data() + offset, sizeof(uint32_t));
    this->segment_message_count_ = segment_message_count;
    offset += sizeof(uint32_t);

    uint64_t max_message_id;

    if (offset + sizeof(uint64_t) > binary_header.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&max_message_id, binary_header.data() + offset, sizeof(uint64_t));
    this->max_message_id_ = max_message_id;
    offset += sizeof(uint64_t);
}


std::vector<std::byte> SegmentHeader::convert_to_bytes() const {
    std::vector<std::byte> buffer;

    // Does not contain the header size

    // file_name doesn't have the .ddb in it
    // Header contents
    append_string_to_buffer(buffer, this->filename_);

    // append schema binary
    // schema size
    append_to_buffer(buffer, static_cast<uint16_t>(this->schema_bytes_.size()));
    // schema bytes
    buffer.insert(buffer.end(), this->schema_bytes_.begin(), this->schema_bytes_.end());

    append_to_buffer(buffer, this->segment_starting_ts_);
    append_to_buffer(buffer, this->segment_ending_ts_);
    append_to_buffer(buffer, this->segment_message_count_);
    append_to_buffer(buffer, this->max_message_id_);

    return buffer;
}

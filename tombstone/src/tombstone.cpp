//
// Created by Arjun on 27/10/2026.
//
#include "tombstone.h"

#include <cstring>
#include <stdexcept>

Tombstone::Tombstone(const std::vector<std::byte>& binary_blob) {
    size_t offset = 0;

    if (binary_blob.size() + offset < (sizeof(int64_t) * 2) + sizeof(uint64_t)) {
        throw std::runtime_error("Corrupted Tombstone");
    }

    std::memcpy(&this->start_time_, binary_blob.data() + offset, sizeof(int64_t));
    offset += sizeof(int64_t);
    std::memcpy(&this->end_time_, binary_blob.data() + offset, sizeof(int64_t));
    offset += sizeof(int64_t);
    std::memcpy(&this->delete_seq_id_, binary_blob.data() + offset, sizeof(uint64_t));
}

bool Tombstone::overlaps(const int64_t q_start_time, const int64_t q_end_time) const {
    return q_start_time <= this->end_time_ && q_end_time >= this->start_time_;
}


std::vector<std::byte> Tombstone::serialize_tombstone() const {
    std::vector<std::byte> buffer;
    const auto* start_ptr = reinterpret_cast<const std::byte*>(&this->start_time_);
    buffer.insert(buffer.end(), start_ptr, start_ptr + sizeof(int64_t));
    const auto* end_ptr = reinterpret_cast<const std::byte*>(&this->end_time_);
    buffer.insert(buffer.end(), end_ptr, end_ptr + sizeof(int64_t));
    const auto* delete_seq_id_ptr = reinterpret_cast<const std::byte*>(&this->delete_seq_id_);
    buffer.insert(buffer.end(), delete_seq_id_ptr, delete_seq_id_ptr + sizeof(uint64_t));

    return buffer;
}

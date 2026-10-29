//
// Created by Arjun on 23/10/2026.
//

#ifndef DUNDERDB_TOMBSTONE_H
#define DUNDERDB_TOMBSTONE_H

#include <vector>
#include <cstdint>

class Tombstone
{
public:
    explicit Tombstone(const int64_t start_time, const int64_t end_time, const uint64_t delete_seq_id) :
    start_time_(start_time), end_time_(end_time), delete_seq_id_(delete_seq_id) {};

    explicit Tombstone(const std::vector<std::byte>& binary_blob);

    [[nodiscard]] bool overlaps(int64_t q_start_time, int64_t q_end_time) const;

    [[nodiscard]] std::vector<std::byte> serialize_tombstone() const;

private:
    int64_t start_time_;
    int64_t end_time_;
    uint64_t delete_seq_id_;
};
#endif //DUNDERDB_TOMBSTONE_H

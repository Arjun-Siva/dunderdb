//
// Created by Arjun on 24/10/2026.
//
#include "cemetery.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <ranges>

void Cemetery::load_cemetery_binary(std::vector<std::byte> binary_blob) {
    size_t offset = 0;
    while (offset < binary_blob.size()) {
        size_t tbs_size;
        if (offset + sizeof(size_t) > binary_blob.size()) {
            throw std::runtime_error("Corrupted Cemetery file");
        }

        std::memcpy(&tbs_size, binary_blob.data() + offset, sizeof(size_t));
        offset += sizeof(size_t);

        if (offset + tbs_size > binary_blob.size()) {
            throw std::runtime_error("Corrupted Cemetery file");
        }

        std::vector<std::byte> tbs(
            binary_blob.begin() + offset,
            binary_blob.begin() + offset + tbs_size
        );

        Tombstone tombstone(tbs);

        this->tombstones_.push_back(tombstone);
        offset += tbs_size;
    }
}

void Cemetery::insert_tombstone(const Tombstone& new_tombstone) {
    // std::unique_lock<std::shared_mutex> lock(this->mutex_);
    // append to the file on disk
    const std::filesystem::path& tbs_file_path = this->tombstone_file_;

    std::ofstream file(
        tbs_file_path,
        std::ios::binary | std::ios::app
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + tbs_file_path.string()
        );
    }

    // Append serialized tombstone
    const std::vector<std::byte> serialized_tbs = new_tombstone.serialize_tombstone();

    file.write(
        reinterpret_cast<const char*>(serialized_tbs.size()),
        sizeof(size_t)
    );

    file.write(
        reinterpret_cast<const char*>(serialized_tbs.data()),
        static_cast<std::streamsize>(serialized_tbs.size())
    );

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + tbs_file_path.string()
        );
    }

    file.close();

    // append in memory
    // no std::move as Tombstone is trivially copyable
    this->tombstones_.push_back(new_tombstone);
}

std::vector<Tombstone> Cemetery::get_overlapping_tombstones(const int64_t query_start_time,
                                                            const int64_t query_end_time) const {
    // std::shared_lock<std::shared_mutex> lock(this->mutex_);
    std::vector<Tombstone> result;

    std::ranges::copy_if(this->tombstones_,
                         std::back_inserter(result),
                         [=](const Tombstone& t) {
                             return t.overlaps(query_start_time, query_end_time);
                         }
    );

    return result;
}

void Cemetery::erase_file_from_disk() const {
    // annihilate on schema drop
    std::filesystem::remove(this->tombstone_file_);
}

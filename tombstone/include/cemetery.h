//
// Created by Arjun on 24/10/2026.
//

#ifndef DUNDERDB_CEMETERY_H
#define DUNDERDB_CEMETERY_H

#include <filesystem>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

#include "tombstone.h"

// List of tombstones of a service
class Cemetery
{
public:
    explicit Cemetery(std::string service_name, std::filesystem::path  cemetery_directory) {
        this->tombstone_file_ = cemetery_directory / std::string(service_name + ".tbs");
        this->cemetery_directory_ = std::move(cemetery_directory);
        this->service_name_ = std::move(service_name);
    }

    void load_cemetery_binary(std::vector<std::byte> binary_blob);
    void insert_tombstone(const Tombstone& new_tombstone);
    [[nodiscard]] std::vector<Tombstone> get_overlapping_tombstones(int64_t query_start_time, int64_t query_end_time) const;
    void erase_file_from_disk() const;

private:
    // mutable std::shared_mutex mutex_;
    std::string service_name_;
    std::filesystem::path cemetery_directory_;
    std::filesystem::path tombstone_file_;
    std::vector<Tombstone> tombstones_;

};

#endif //DUNDERDB_CEMETERY_H

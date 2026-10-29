//
// Created by Arjun on 24/10/2026.
//

#ifndef DUNDERDB_SEXTON_H
#define DUNDERDB_SEXTON_H

#include <unordered_map>

#include "cemetery.h"
#include "lock_map.h"

class Sexton
{
    public:
    explicit Sexton(const std::filesystem::path& cemetery_directory) : cemetery_directory_(cemetery_directory) {};
    void add_new_cemetery(const std::string& service_name);
    void add_new_cemetery(const std::string& service_name, Cemetery service_cemetery);
    void drop_cemetery(const std::string& service_name);
    void push_tombstone_to_cemetery(const std::string& service_name, const Tombstone& tombstone);
    std::vector<Tombstone> get_overlapping_tombstones_from_cemetery(const std::string& service_name, int64_t start_time, int64_t end_time);

    private:
    LockMap lock_map_;
    std::filesystem::path cemetery_directory_;
    std::unordered_map<std::string, std::shared_ptr<Cemetery>> cemetery_map_;
};

#endif //DUNDERDB_SEXTON_H

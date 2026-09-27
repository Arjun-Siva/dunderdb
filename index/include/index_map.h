//
// Created by Arjun on 02/08/2026.
//

#ifndef DUNDERDB_INDEX_MAP_H
#define DUNDERDB_INDEX_MAP_H
#include <memory>
#include <string>
#include <unordered_map>

#include "service_index.h"

class IndexMap {
public:
    IndexMap() = default;
    void add_index(const std::string& index_name, std::shared_ptr<ServiceIndex> index);
    void delete_index(const std::string& index_name);
    [[nodiscard]] std::shared_ptr<ServiceIndex> get_index(const std::string& index_name) const;
    [[nodiscard]] bool contains(const std::string& index_name) const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<ServiceIndex>> indexes_;
};

#endif //DUNDERDB_INDEX_MAP_H

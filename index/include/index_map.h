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
    void add_index(const std::string& index_name, std::unique_ptr<ServiceIndex> index);
    ServiceIndex& get_index(const std::string& index_name) const;
    bool contains(const std::string& index_name) const;

private:
    std::unordered_map<std::string, std::unique_ptr<ServiceIndex>> indexes_;
};

#endif //DUNDERDB_INDEX_MAP_H

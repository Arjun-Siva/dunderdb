//
// Created by Arjun on 02/08/2026.
//
#include "index_map.h"

void IndexMap::add_index(const std::string &index_name, std::unique_ptr<ServiceIndex> index) {
    this->indexes_.emplace(index_name, std::move(index));
}

ServiceIndex& IndexMap::get_index(const std::string &index_name) const {
    return *this->indexes_.at(index_name);
}

bool IndexMap::contains(const std::string &index_name) const {
    return this->indexes_.contains(index_name);
}

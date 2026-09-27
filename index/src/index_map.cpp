//
// Created by Arjun on 02/08/2026.
//
#include "index_map.h"

#include <mutex>

void IndexMap::add_index(const std::string &index_name, std::shared_ptr<ServiceIndex> index) {
    std::unique_lock lock(this->mutex_);
    this->indexes_.emplace(index_name, std::move(index)); // only the pointer is moved in, not the object itself
}

void IndexMap::delete_index(const std::string& index_name) {
    std::unique_lock lock(this->mutex_);
    this->indexes_.erase(index_name); // the actual object is destroyed only when all shared_ptrs are destroyed
}

std::shared_ptr<ServiceIndex> IndexMap::get_index(const std::string &index_name) const {
    std::shared_lock lock(this->mutex_);
    return this->indexes_.at(index_name); // returns a copy of the shared_ptr owned by the map, not the object itself
}

bool IndexMap::contains(const std::string &index_name) const {
    std::shared_lock lock(this->mutex_);
    return this->indexes_.contains(index_name);
}

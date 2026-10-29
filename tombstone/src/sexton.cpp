//
// Created by Arjun on 29/10/2026.
//
#include "sexton.h"

void Sexton::add_new_cemetery(const std::string& service_name) {
    this->lock_map_.create(service_name);
    std::unique_lock<std::shared_mutex> lock = this->lock_map_.get_exclusive_lock(service_name);
    this->cemetery_map_.emplace(service_name, std::make_shared<Cemetery>(service_name, this->cemetery_directory_));
}

void Sexton::add_new_cemetery(const std::string& service_name, Cemetery service_cemetery) {
    this->lock_map_.create(service_name);
    std::unique_lock<std::shared_mutex> lock = this->lock_map_.get_exclusive_lock(service_name);
    this->cemetery_map_.emplace(service_name, std::make_shared<Cemetery>(std::move(service_cemetery)));
}

void Sexton::drop_cemetery(const std::string& service_name) {
    std::unique_lock<std::shared_mutex> lock = this->lock_map_.get_exclusive_lock(service_name);
    this->cemetery_map_.erase(service_name);
}

void Sexton::push_tombstone_to_cemetery(const std::string& service_name, const Tombstone& tombstone) {
    std::unique_lock<std::shared_mutex> lock = this->lock_map_.get_exclusive_lock(service_name);
    const std::shared_ptr<Cemetery> cemetery_ptr = this->cemetery_map_.at(service_name);
    cemetery_ptr->insert_tombstone(tombstone);
}

std::vector<Tombstone> Sexton::get_overlapping_tombstones_from_cemetery(const std::string& service_name,
    const int64_t start_time, const int64_t end_time) {
    std::shared_lock<std::shared_mutex> lock = this->lock_map_.get_shared_lock(service_name);
    const std::shared_ptr<Cemetery> cemetery_ptr = this->cemetery_map_.at(service_name);
    std::vector<Tombstone> result_tbs = cemetery_ptr->get_overlapping_tombstones(start_time, end_time);
    return result_tbs;
}


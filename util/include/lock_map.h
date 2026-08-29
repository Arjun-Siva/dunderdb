//
// Created by Arjun on 28/08/2026.
//

#ifndef DUNDERDB_LOCK_MAP_H
#define DUNDERDB_LOCK_MAP_H

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <stdexcept>

class LockMap {
public:
    LockMap() = default;

    LockMap(const LockMap&) = delete;
    LockMap& operator=(const LockMap&) = delete;

    LockMap(LockMap&&) = delete;
    LockMap& operator=(LockMap&&) = delete;

    /**
     * Creates a new lock associated with 'key'.
     *
     * Throws std::invalid_argument if the key already exists.
     */
    void create(const std::string& key) {
        std::lock_guard<std::mutex> lock(map_mutex_);

        auto [_it, inserted] =
            locks_.try_emplace(key, std::make_shared<std::shared_mutex>());

        if (!inserted) {
            throw std::invalid_argument(
                "LockMap: lock already exists for key: " + key
            );
        }
    }

    /**
     * Acquires a shared/read lock for 'key'.
     *
     * Throws std::out_of_range if the key does not exist.
     *
     * The returned lock is released automatically when it goes
     * out of scope.
     */
    std::shared_lock<std::shared_mutex> get_shared_lock(const std::string& key) {
        const std::shared_ptr<std::shared_mutex> mutex = get_mutex(key);
        return std::shared_lock<std::shared_mutex>(*mutex);
    }

    /**
     * Acquires an exclusive/write lock for 'key'.
     *
     * Throws std::out_of_range if the key does not exist.
     *
     * The returned lock is released automatically when it goes
     * out of scope.
     */
    std::unique_lock<std::shared_mutex> get_exclusive_lock(const std::string& key) {
        const std::shared_ptr<std::shared_mutex> mutex = get_mutex(key);
        return std::unique_lock<std::shared_mutex>(*mutex);
    }

private:
    std::shared_ptr<std::shared_mutex> get_mutex(const std::string& key) {
        std::lock_guard<std::mutex> lock(map_mutex_);

        const auto it = locks_.find(key);

        if (it == locks_.end()) {
            throw std::out_of_range(
                "LockMap: no lock exists for key: " + key
            );
        }

        return it->second;
    }

    // Protects the lock registry itself.
    std::mutex map_mutex_;

    // Lock objects are intentionally never removed.
    //
    // The map owns the mutex for the lifetime of the LockMap.
    // This guarantees that a key always refers to the same mutex.
    std::unordered_map<
        std::string,
        std::shared_ptr<std::shared_mutex>
    > locks_;
};
#endif //DUNDERDB_LOCK_MAP_H

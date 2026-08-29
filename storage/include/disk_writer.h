//
// Created by Arjun on 29/07/2026.
//

#ifndef DUNDERDB_DISK_WRITER_H
#define DUNDERDB_DISK_WRITER_H

#include <filesystem>

#include "common_queue.h"
#include "segment_metadata.h"
#include "flush_job.h"
#include "index_map.h"
#include "lock_map.h"

// pops a flush job from disk queue
// serialize messages
// if NEW, create a new file seg_servicename.tmp, writer header, append the records
// if APPEND, append records
// if SEAL, append records, rename seg_servicename.tmp to seg_servicename_timestamp.ddb, update indexes
class DiskWriter {
public:
    DiskWriter() = delete;

    explicit DiskWriter(CommonQueue<FlushJob> &disk_queue, const std::string &files_directory,
                        IndexMap &index_map, LockMap &tmp_file_lock_map) : services_directory_(files_directory),
                                                                           disk_queue_(disk_queue),
                                                                           index_map_(index_map),
                                                                           tmp_file_lock_map_(tmp_file_lock_map) {
    };

    void start();

    void join();

private:
    std::filesystem::path services_directory_;
    CommonQueue<FlushJob> &disk_queue_;
    IndexMap &index_map_;
    LockMap &tmp_file_lock_map_;

    std::thread thread_;

    void run() const;
};

#endif //DUNDERDB_DISK_WRITER_H

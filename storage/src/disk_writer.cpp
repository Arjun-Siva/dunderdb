//
// Created by Arjun on 31/07/2026.
//

#include "disk_writer.h"
#include "message_serializer.h"
#include "segment_metadata.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

void append_to_segment_file(const std::filesystem::path& services_directory,
                            const std::string& service_name,
                            std::vector<std::byte> messages_bytes
) {
    const std::filesystem::path service_directory =
        services_directory / service_name;

    const std::filesystem::path file_path =
        service_directory / std::string("seg_" + service_name + ".tmp");

    // directory is assumed to be present already, bytes APPENDED to existing file

    std::ofstream file(
        file_path,
        std::ios::binary | std::ios::app
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + file_path.string()
        );
    }

    // Append serialized records
    file.write(
        reinterpret_cast<const char*>(messages_bytes.data()),
        static_cast<std::streamsize>(messages_bytes.size())
    );

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + file_path.string()
        );
    }
}

void create_segment_file(const std::filesystem::path& services_directory,
                         const std::string& service_name,
                         std::vector<std::byte> header_bytes,
                         std::vector<std::byte> messages_bytes
) {
    // Create /services/service_name directory if it doesn't exist
    const std::filesystem::path service_directory =
        services_directory / service_name;

    std::filesystem::create_directories(service_directory);

    // Create NEW segment file
    std::filesystem::path file_path =
        service_directory / std::string("seg_" + service_name + ".tmp");

    std::ofstream file(
        file_path,
        std::ios::binary | std::ios::trunc
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + file_path.string()
        );
    }

    // Append header
    file.write(
        reinterpret_cast<const char*>(header_bytes.data()),
        static_cast<std::streamsize>(header_bytes.size())
    );

    // Append serialized records
    file.write(
        reinterpret_cast<const char*>(messages_bytes.data()),
        static_cast<std::streamsize>(messages_bytes.size())
    );
    std::cout << "Created file: " << file_path.string() << std::endl;

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + file_path.string()
        );
    }
}

void rename_segment_file(const std::filesystem::path& services_directory,
                         const std::string& service_name,
                         const std::string& old_file_name,
                         const std::string& new_file_name) {
    const std::filesystem::path service_directory = services_directory / service_name;
    const std::filesystem::path old_path = service_directory / old_file_name;
    const std::filesystem::path new_path = service_directory / new_file_name;

    try {
        std::filesystem::rename(old_path, new_path);
        std::cout << "Renamed file: " << old_path.string() << " to " << new_path.string() << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(
            "Failed to rename " +
            old_path.string() +
            " to " +
            new_path.string() +
            ": " +
            e.what()
        );
    }
}

std::vector<std::byte> serialize_messages_vector(const std::vector<ValidatedMessage>& messages) {
    std::vector<std::byte> buffer;

    for (const auto& message : messages) {
        auto msg_bytes = MessageSerializer::serialize_message(message);
        // msg_bytes includes the binary message size as well
        buffer.insert(buffer.end(), msg_bytes.begin(), msg_bytes.end());
    }

    return buffer;
}

void DiskWriter::run() const {
    // pop a job
    // when flush_job doesn't have value, the disk job queue is closed, and diskwriter can exit
    while (auto flush_job = this->disk_queue_.dequeue()) {
        auto [type, service_name, messages, file_name,
            header_binary,starting_ts, ending_ts, count] = flush_job.value();

        auto messages_binary = serialize_messages_vector(messages);

        // identify the type of job
        switch (type) {
        case APPEND:
            {
                {
                    std::unique_lock<std::shared_mutex> lock = this->tmp_file_lock_map_.
                                                                     get_exclusive_lock(service_name);
                    append_to_segment_file(this->services_directory_, service_name, std::move(messages_binary));
                }
                break;
            }
        case NEW:
            {
                // file_name doesn't have the .ddb in it
                {
                    std::vector<std::byte> header_bytes = MessageSerializer::generate_segment_header(file_name, header_binary);
                    std::unique_lock<std::shared_mutex> lock = this->tmp_file_lock_map_.
                                                                     get_exclusive_lock(service_name);
                    create_segment_file(this->services_directory_, service_name, std::move(header_bytes),
                                        std::move(messages_binary));
                }

                break;
            }
        case SEAL:
            {
                // append messages to disk, rename file
                {
                    std::unique_lock<std::shared_mutex> lock = this->tmp_file_lock_map_.
                                                                     get_exclusive_lock(service_name);
                    append_to_segment_file(this->services_directory_, service_name, std::move(messages_binary));
                    rename_segment_file(
                        this->services_directory_,
                        service_name,
                        std::string("seg_" + service_name + ".tmp"),
                        std::string(file_name + ".ddb")
                    );
                }

                // update indexes
                // NOTE: metadata is std::moved by the end of append_segment_metadata
                SegmentMetadata metadata{
                    starting_ts, ending_ts, count, std::string(file_name + ".ddb")
                };

                const std::shared_ptr<ServiceIndex> serv_index = this->index_map_.get_index(service_name);
                serv_index->append_segment_metadata(metadata);
                break;
            }
        case NEW_SEAL:
            {
                {
                    std::vector<std::byte> header_bytes = MessageSerializer::generate_segment_header(file_name, header_binary);
                    std::unique_lock<std::shared_mutex> lock = this->tmp_file_lock_map_.
                                                                     get_exclusive_lock(service_name);
                    // first create a tmp file like normal NEW
                    create_segment_file(this->services_directory_, service_name, std::move(header_bytes),
                                        std::move(messages_binary));
                    // immediately rename
                    rename_segment_file(
                        this->services_directory_,
                        service_name,
                        std::string("seg_" + service_name + ".tmp"),
                        std::string(file_name + ".ddb")
                    );
                }

                // update indexes
                // NOTE: metadata is std::moved by the end of append_segment_metadata
                SegmentMetadata metadata{
                    starting_ts, ending_ts, count, std::string(file_name + ".ddb")
                };

                std::shared_ptr<ServiceIndex> serv_index = this->index_map_.get_index(service_name);
                serv_index->append_segment_metadata(metadata);
                break;
            }
        case DROP_SERVICE:
            break;
        case DELETE:
            break;
        } // switch-end
    } // while-end
}

void DiskWriter::start() {
    this->thread_ = std::thread(&DiskWriter::run, this);
}

void DiskWriter::join() {
    if (this->thread_.joinable()) this->thread_.join();
}

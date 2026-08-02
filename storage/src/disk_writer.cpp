//
// Created by Arjun on 31/07/2026.
//

#include "disk_writer.h"
#include "serializer.h"
#include "segment_metadata.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

void append_to_segment_file(const std::filesystem::path& services_directory,
                         const std::string& service_name,
                         const std::vector<std::byte>& messages_bytes
                         ) {
    const std::filesystem::path service_directory =
        services_directory / service_name;

    const std::filesystem::path file_path =
        service_directory / std::string("seg_" + service_name + ".tmp");

    // directory is assumed to be present already

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
                         const std::vector<std::byte>& header_bytes,
                         const std::vector<std::byte>& messages_bytes
) {
    // Create /services/service_name directory if it doesn't exist
    const std::filesystem::path service_directory =
        services_directory / service_name;

    std::filesystem::create_directories(service_directory);

    // Create/open segment file
    std::filesystem::path file_path =
        service_directory / std::string("seg_" + service_name + ".tmp");

    std::ofstream file(
        file_path,
        std::ios::binary | std::ios::app
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
    std::cout<<"Created file: "<<file_path.string()<<std::endl;

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + file_path.string()
        );
    }
}

void rename_segment_file(
    const std::filesystem::path& services_directory,
    const std::string& service_name,
    const std::string& old_file_name,
    const std::string& new_file_name)
{
    std::filesystem::path service_directory =
        services_directory / service_name;

    const std::filesystem::path old_path =
        service_directory / old_file_name;

    const std::filesystem::path new_path =
        service_directory / new_file_name;

    try
    {
        std::filesystem::rename(old_path, new_path);
        std::cout<<"Renamed file: "<<old_path.string()<<" to "<<new_path.string()<<std::endl;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
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
        auto msg_bytes = Serializer::serialize_message(message);
        buffer.insert(buffer.end(), msg_bytes.begin(), msg_bytes.end());
    }

    return buffer;
}

void DiskWriter::run() const {
    // pop a job
    while (true) {
        auto [type, service_name, messages, file_name] = this->disk_queue_.dequeue();
        const auto first_message = messages.front();
        const auto last_message = messages.back();
        const uint32_t count = messages.size();

        auto messages_binary = serialize_messages_vector(messages);

        std::cout<<"Diskwriter writes!!"<<std::endl;

        // identify the type of job
        switch (type) {
            case APPEND: {
                append_to_segment_file(this->services_directory_, service_name, std::move(messages_binary));
                break;
            }
            case NEW: {
                std::vector<std::byte> header_bytes = Serializer::generate_segment_header(file_name);
                create_segment_file(this->services_directory_, service_name, std::move(header_bytes),
                                    std::move(messages_binary));
                break;
            }
            case SEAL: {
                // append messages to disk, rename file
                append_to_segment_file(this->services_directory_, service_name, std::move(messages_binary));
                rename_segment_file(
                    this->services_directory_,
                    service_name,
                    std::string("seg_" + service_name + ".tmp"),
                    std::string(file_name + ".ddb")
                );

                // update indexes
                SegmentMetadata metadata{
                    first_message.timestamp, last_message.timestamp, count, std::string(file_name + ".ddb")
                };

                ServiceIndex& serv_index = this->index_map_.get_index(service_name);
                serv_index.append_segment_metadata(metadata);
                break;
            }
        }
    }
}

void DiskWriter::start() {
    this->thread_ = std::thread(&DiskWriter::run, this);
}

void DiskWriter::join() {
    if (this->thread_.joinable()) this->thread_.join();
}
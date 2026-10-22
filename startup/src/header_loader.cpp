//
// Created by Arjun on 22/10/2026.
//
#include "header_loader.h"
#include "file_loader.h"

SegmentHeader HeaderLoader::load_segment_header(const std::filesystem::path& services_directory,
                                                const std::string& service_name, const std::string& file_name) {
    const std::filesystem::path file_path = services_directory / service_name / file_name;

    std::ifstream file(file_path, std::ios::binary | std::ios::ate);

    if (!file) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }

    // returns last position in the file since ios::ate flag goes to the end
    const std::ifstream::pos_type file_size = file.tellg();

    if (file_size == std::ifstream::pos_type(-1)) {
        throw std::runtime_error("Failed to determine file size: " + file_path.string());
    }

    const auto size = static_cast<std::size_t>(file_size);

    // header_size (uint32) | header_binary | messages ....
    if (size < sizeof(uint32_t)) {
        throw std::runtime_error("Corrupted file");
    }

    // move to the beginning of the file
    file.seekg(0);

    uint32_t header_size;
    if (!file.read(reinterpret_cast<char*>(&header_size), sizeof(header_size))) {
        throw std::runtime_error("Failed to read uint32_t from file");
    }

    if (size < sizeof(uint32_t) + header_size) {
        throw std::runtime_error("Corrupted file");
    }

    // The file pointer automatically advances after each read(), so no need seekg() here
    std::vector<std::byte> bytes_vector(header_size);
    if (!file.read(reinterpret_cast<char*>(bytes_vector.data()), static_cast<std::streamsize>(header_size))) {
        throw std::runtime_error("Failed to read header from file");
    }

    return SegmentHeader{bytes_vector};
}

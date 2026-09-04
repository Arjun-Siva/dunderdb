//
// Created by Arjun on 11/08/2026.
//

#ifndef DUNDERDB_FILE_LOADER_H
#define DUNDERDB_FILE_LOADER_H

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

struct LoadedFile {
    std::string filename;
    std::vector<std::byte> bytes;
};

class FileLoader {
public:
    static std::vector<LoadedFile> load_directory(const std::filesystem::path& directory) {
        if (!std::filesystem::exists(directory)) {
            throw std::runtime_error(
                "Directory does not exist: " + directory.string()
            );
        }

        if (!std::filesystem::is_directory(directory)) {
            throw std::runtime_error(
                "Path is not a directory: " + directory.string()
            );
        }

        std::vector<LoadedFile> files;

        for (const std::filesystem::directory_entry& entry :
             std::filesystem::directory_iterator(directory))
        {
            if (!entry.is_regular_file()) {
                continue;
            }

            LoadedFile loaded_file;

            loaded_file.filename = entry.path().filename().string();
            loaded_file.bytes = read_file(entry.path());

            files.push_back(std::move(loaded_file));
        }

        return files;
    }

    static std::vector<std::byte> read_file(const std::filesystem::path& file_path) {
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

        std::vector<std::byte> bytes_vector(size);

        // move to the beginning of the file
        file.seekg(0);

        if (size > 0) {
            file.read(
                reinterpret_cast<char*>(bytes_vector.data()),
                static_cast<std::streamsize>(size) // unsigned long is converted to signed long, but nothing to worry about
            );

            if (!file) {
                throw std::runtime_error(
                    "Failed to read file: " + file_path.string()
                );
            }
        }

        return bytes_vector;
    }
};

#endif //DUNDERDB_FILE_LOADER_H

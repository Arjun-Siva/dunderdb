//
// Created by Arjun on 11/08/2026.
//

#ifndef DUNDERDB_FILE_LOADER_H
#define DUNDERDB_FILE_LOADER_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

struct LoadedFile {
    std::string filename;
    std::vector<std::byte> bytes;
};

class FileLoader {
public:
    static std::vector<LoadedFile> load_directory(const std::filesystem::path& directory);

private:
    static std::vector<std::byte> read_file(const std::filesystem::path& file_path);
};

#endif //DUNDERDB_FILE_LOADER_H

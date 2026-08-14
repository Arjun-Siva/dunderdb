//
// Created by Arjun on 13/08/2026.
//
#include "index_loader.h"
#include "file_loader.h"
#include "index_serializer.h"

IndexMap IndexLoader::load_indexes(const std::filesystem::path &index_directory) {
    // fetch binary data from files in the directory
    std::vector<LoadedFile> files = FileLoader::load_directory(index_directory);
    IndexMap map;

    for (const auto&[filename, bytes] : files) {
        std::string file_name = filename;
        if (file_name.size() < 4 || file_name.substr(file_name.size() - 4) != ".idx") {
            throw std::runtime_error("Invalid index filename: " + file_name);
        }

        std::string service_name = file_name.substr(0, file_name.size() - 4);

        // deserialize the binary blobs
        std::vector<SegmentMetadata> metadata_index_entries = IndexSerializer::deserialize_bytes_of_index_file(bytes);

        std::unique_ptr<ServiceIndex> service_index_ptr = std::make_unique<ServiceIndex>(service_name, index_directory);
        service_index_ptr->load_segment_metadata_list(metadata_index_entries);

        // add to index map
        map.add_index(service_name, std::move(service_index_ptr));
    }

    return map;
}

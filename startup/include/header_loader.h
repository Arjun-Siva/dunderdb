//
// Created by Arjun on 22/10/2026.
//

#ifndef DUNDERDB_HEADER_LOADER_H
#define DUNDERDB_HEADER_LOADER_H
#include <filesystem>

#include "segment_header.h"

class HeaderLoader
{
    public:
        static SegmentHeader load_segment_header(const std::filesystem::path& services_directory, const std::string& service_name, const std::string& file_name);
};
#endif //DUNDERDB_HEADER_LOADER_H

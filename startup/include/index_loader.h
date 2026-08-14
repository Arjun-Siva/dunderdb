//
// Created by Arjun on 11/08/2026.
//

#ifndef DUNDERDB_INDEX_LOADER_H
#define DUNDERDB_INDEX_LOADER_H

#include <filesystem>

#include "index_map.h"

class IndexLoader {
    public:
        static IndexMap load_indexes(const std::filesystem::path& index_directory);
};
#endif //DUNDERDB_INDEX_LOADER_H

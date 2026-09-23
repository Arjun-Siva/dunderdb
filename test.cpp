#include <iostream>
#include <cassert>

#include "common_queue.h"
#include "ingestion_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"
#include "service_buffer.h"
#include "buffer_map.h"
#include "disk_writer.h"
#include "schema_serializer.h"
#include "query/include/segment.h"
#include "select_handler.h"
#include "time_converter.h"

void write_schema_to_disk(std::string&& name, std::vector<std::byte> schema_bytes, const std::string& schemas_directory) {
    const std::filesystem::path schema_file_path = schemas_directory + "/" + name+ ".sch";

    std::ofstream file(
        schema_file_path,
        std::ios::binary | std::ios::trunc
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + schema_file_path.string()
        );
    }

    // Append serialized indexes metadata
    file.write(
        reinterpret_cast<const char*>(schema_bytes.data()),
        static_cast<std::streamsize>(schema_bytes.size())
    );

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + schema_file_path.string()
        );
    }
}

int main() {
    const std::string original = "2026-09-17T10:23:45Z";

    const int64_t epoch_ms =
        TimeConverter::utc_to_epoch_ms(original);

    const std::string result =
        TimeConverter::epoch_ms_to_iso(epoch_ms);

    std::cout << "Original: " << original << '\n';
    std::cout << "Epoch:    " << epoch_ms << '\n';
    std::cout << "Result:   " << result << '\n';

    // assert(result == original);

    std::cout << "Round trip OK\n";

    // 1790164103845
    // 1790164112568

    // 1790164173913
    // 1790164173918
    const int64_t ms = 1790164112568;
    std::cout<< "Converted ms:" <<TimeConverter::epoch_ms_to_iso(ms);

    return 0;
}

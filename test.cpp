#include <iostream>

#include "common_queue.h"
#include "ingestion_receiver.h"
#include "schema.h"
#include "unvalidated_message.h"
#include "validator.h"
#include "service_buffer.h"
#include "buffer_map.h"
#include "disk_writer.h"
#include "schema_serializer.h"

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
    std::cout << "Starting __DunderDB__" << std::endl;
    CommonQueue<UnvalidatedMessage> insertion_queue;

    // load schemas
    Schema sales("sales");
    sales.add_column(Column("item", ColumnType::STRING, 20, false) );
    sales.add_column(Column("price", ColumnType::NUMBER, false) );
    sales.add_column(Column("category", ColumnType::STRING, 10, true));

    // serialize to disk
    std::vector<std::byte> schema_bytes = SchemaSerializer::serialize_schema(sales);
    std::string schemas_directory = "/home/arjunsiva/dunderdb/data/schemas";
    write_schema_to_disk("sales", schema_bytes, schemas_directory);

    Schema employee("employee");
    employee.add_column(Column("name", ColumnType::STRING, 20, false) );
    employee.add_column(Column("age", ColumnType::NUMBER, true) );
    employee.add_column(Column("department", ColumnType::STRING, 10, true));

    // serialize to disk
    schema_bytes = SchemaSerializer::serialize_schema(employee);
    write_schema_to_disk("employee", schema_bytes, schemas_directory);
    return 0;
}

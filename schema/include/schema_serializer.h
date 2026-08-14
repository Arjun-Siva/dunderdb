//
// Created by Arjun on 06/08/2026.
//

#ifndef DUNDERDB_SCHEMA_SERIALIZER_H
#define DUNDERDB_SCHEMA_SERIALIZER_H

#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <vector>
#include "column.h"
#include "schema.h"

class SchemaSerializer {
    public:
    static std::vector<std::byte> serialize_column(const Column& column) {
        // | column name size | column name | column type | max characters | nullable |
        std::vector<std::byte> buffer;
        append_string_to_buffer(buffer, column.get_name());

        const uint8_t col_type = static_cast<uint8_t>(column.get_column_type());
        append_to_buffer(buffer, col_type);

        const uint16_t max_chars = column.get_max_characters();
        append_to_buffer(buffer, max_chars);

        const uint8_t nullable = column.get_nullable() ? 1 : 0;
        append_to_buffer(buffer, nullable);

        return buffer;
    }

    static Column deserialize_bytes_to_column(const std::span<const std::byte>& bytes) {
        size_t offset = 0;

        auto read = [&](auto& value) {
            // dst, src, size
            std::memcpy(&value, bytes.data() + offset, sizeof(value));
            offset += sizeof(value);
        };

        uint16_t column_name_size;
        read(column_name_size);

        std::string column_name;
        column_name.resize(column_name_size);
        std::memcpy(column_name.data(), bytes.data() + offset, column_name_size);
        offset += column_name_size;

        uint8_t column_type;
        read(column_type);

        uint16_t max_characters;
        read(max_characters);

        uint8_t nullable;
        read(nullable);

        if (column_type == 1) {
            // string
            Column column(column_name, ColumnType::STRING, max_characters, nullable);
            return column;
        } //else {
            // number
            Column column(column_name, ColumnType::NUMBER, nullable);
            return column;
        // }
    }

    static std::vector<std::byte> serialize_schema(const Schema& schema) {
        // | schema name size | schema name | column size | column | column size| ... | column |
        std::vector<std::byte> buffer;
        const std::string schema_name = schema.get_service_name();
        append_string_to_buffer(buffer, schema_name);

        for (const Column& column : schema.get_columns_in_order()) {
            std::vector<std::byte> bytes = serialize_column(column);
            uint16_t col_size = static_cast<uint16_t>(bytes.size());
            append_to_buffer(buffer, col_size);
            buffer.insert(buffer.end(), bytes.begin(), bytes.end());
        }

        return buffer;
    }

    static Schema deserialize_bytes_to_schema(const std::vector<std::byte>& bytes_vector) {
        // service name size
        size_t offset = 0;

        uint16_t service_name_size; // in bytes or number of chars
        std::string service_name;

        if (offset + sizeof(uint16_t) > bytes_vector.size())
            throw std::runtime_error("Corrupted Schema file");

        std::memcpy(&service_name_size,
                        bytes_vector.data() + offset,
                        sizeof(service_name_size));

        offset += sizeof(service_name_size);

        // service name string
        if (offset + service_name_size > bytes_vector.size())
            throw std::runtime_error("Corrupted Schema file");

        service_name.resize(service_name_size);

        std::memcpy(service_name.data(), bytes_vector.data() + offset, service_name_size);
        offset += service_name_size;

        Schema schema_object(service_name);

        // columns
        while (offset < bytes_vector.size()) {
            uint16_t column_object_size;

            if (offset + sizeof(uint16_t) > bytes_vector.size())
                throw std::runtime_error("Corrupted Schema file");

            std::memcpy(&column_object_size,
              bytes_vector.data() + offset,
                sizeof(column_object_size));

            offset += sizeof(column_object_size);

            if (offset + column_object_size > bytes_vector.size())
                throw std::runtime_error("Corrupted Schema file");

            auto column_object = deserialize_bytes_to_column(std::span<const std::byte>(
                bytes_vector.data() + offset,
                column_object_size));

            schema_object.add_column(column_object);

            offset += column_object_size;
        }

        return schema_object;
    }

private:
    // maybe move this into a separate file in util and share with other serializers
    template<typename T>
    static void append_to_buffer(std::vector<std::byte>& buffer, const T& value) {
        // checked during compile time if T is POD
        static_assert(std::is_trivially_copyable_v<T>);
        // convert all types to array of bytes
        const auto* ptr = reinterpret_cast<const std::byte*>(&value);
        // append the array of bytes to the vector
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    static void append_string_to_buffer(std::vector<std::byte>& buffer, const std::string& value)
    {
        // uint16_t can represent ~ 65K
        const uint16_t size = static_cast<uint16_t>(value.size());
        append_to_buffer(buffer, size);
        const auto* ptr = reinterpret_cast<const std::byte*>(value.data());
        buffer.insert(buffer.end(), ptr, ptr + size);
    }
};

#endif //DUNDERDB_SCHEMA_SERIALIZER_H

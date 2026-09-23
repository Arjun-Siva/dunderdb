//
// Created by Arjun on 29/07/2026.
//

#ifndef DUNDERDB_SERIALIZER_H
#define DUNDERDB_SERIALIZER_H
#include <bitset>
#include <vector>
#include <cstring>
#include <span>

#include "validated_message.h"

class MessageSerializer
{
public:
    static std::vector<std::byte> serialize_message(const ValidatedMessage& message) {
        // Includes the message size - Message size(uint32) | message blob
        std::vector<std::byte> buffer;

        // Reserve a reasonable amount to reduce reallocations, this doesn't include the string sizes
        buffer.reserve(message.estimated_size + sizeof(uint32_t));

        // add a placeholder for the total message size
        // 4 bytes unsigned
        append_to_buffer(buffer, uint32_t{0});

        // timestamp is fixed size int64_t in all messages
        append_to_buffer(buffer, message.timestamp);

        // Build NULL bitmap, 8 bytes fixed. Max columns - 64
        std::bitset<64> bitmap;

        for (size_t i = 0; i < message.records.size(); ++i) {
            if (message.records[i].has_value()) {
                bitmap.set(i);
            }
        }

        // NOTE: while reading this value use 'unsigned long long',
        // instead of fixed size types, as it could be platform dependent
        append_to_buffer(buffer, bitmap.to_ullong());

        // Serialize values.
        for (const auto& field : message.records) {
            if (!field.has_value()) {
                continue;
            }

            const Value& value = field.value();

            if (std::holds_alternative<double>(value)) {
                // we don't append the size of numerical values
                // as the order of columns is deduced from the null map and schema
                append_to_buffer(buffer, std::get<double>(value));
            }
            else {
                const auto& str = std::get<std::string>(value);
                append_string_to_buffer(buffer, str);
            }
        }

        // Overwrite record placeholder size
        // since the buffer vector uses one byte per element, the size in bytes is simply the number of elements
        const uint32_t record_size = static_cast<uint32_t>(buffer.size() - sizeof(uint32_t));
        std::memcpy(buffer.data(), &record_size, sizeof(record_size));

        return buffer;
    }

    static ValidatedMessage deserialize_message(const Schema& schema, const std::span<const std::byte> bytes_vector) {
        // doesn't contain the size of the message
        // timestamp(int64) | null bitmap (unsigned long long) | Values ...
        // Values = Double or (String length | String value)
        size_t offset = 0;
        int64_t timestamp;

        if (offset + sizeof(int64_t) > bytes_vector.size()) {
            throw std::runtime_error("Corrupted segment file");
        }

        std::memcpy(&timestamp, bytes_vector.data() + offset, sizeof(int64_t));
        offset += sizeof(int64_t);

        if (offset + sizeof(unsigned long long) > bytes_vector.size()) {
            throw std::runtime_error("Corrupted segment file");
        }
        unsigned long long bitmap_value;

        std::memcpy(&bitmap_value, bytes_vector.data() + offset, sizeof(unsigned long long));
        offset += sizeof(unsigned long long);

        const std::bitset<64> bitmap{bitmap_value};

        // Loop through columns in the schema, find their types
        const std::vector<Column> columns = schema.get_columns_in_order();
        // RecordsVector accepts double or string or nullopt
        RecordsVector message_values;

        for (size_t i = 0; i < columns.size(); ++i) {
            const ColumnType type = columns[i].get_column_type();

            if (!bitmap.test(i)) {
                message_values.emplace_back(std::nullopt);
                continue;
            }

            if (type == ColumnType::NUMBER) {
                double number;

                if (offset + sizeof(double) > bytes_vector.size())
                    throw std::runtime_error("Corrupted message");

                std::memcpy(&number, bytes_vector.data() + offset, sizeof(double));
                offset += sizeof(double);

                message_values.emplace_back(number);
            }

            else if (type == ColumnType::STRING) {
                // String length(uint32) | String value
                uint32_t string_size;
                if (offset + sizeof(uint32_t) > bytes_vector.size()) {
                    throw std::runtime_error("Corrupted message");
                }

                std::memcpy(&string_size, bytes_vector.data() + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);

                if (offset + string_size > bytes_vector.size()) {
                    throw std::runtime_error("Corrupted message");
                }

                std::string string_value(
                    reinterpret_cast<const char*>(bytes_vector.data() + offset),
                    string_size
                );
                offset += string_size;

                message_values.emplace_back(string_value);
            }
        }

        return ValidatedMessage{message_values, timestamp};
    }

    static std::vector<std::byte> generate_segment_header(const std::string& filename, const std::vector<std::byte>& header) {
        std::vector<std::byte> buffer;

        // Reserve space for header size
        append_to_buffer(buffer, uint32_t{0});

        // file_name doesn't have the .ddb in it
        // Header contents
        append_string_to_buffer(buffer, filename);

        // append schema binary
        // schema size
        append_to_buffer(buffer, static_cast<uint32_t>(header.size()));
        // schema bytes
        // FUTURE: think about using std::move
        buffer.insert(buffer.end(), header.begin(), header.end());

        // Patch header size (excluding the size field itself)
        const uint32_t header_size = static_cast<uint32_t>(buffer.size() - sizeof(uint32_t));

        std::memcpy(
            buffer.data(),
            &header_size,
            sizeof(header_size)
        );

        return buffer;
    }

private:
    template <typename T>
    static void append_to_buffer(std::vector<std::byte>& buffer, const T& value) {
        // checked during compile time if T is POD
        static_assert(std::is_trivially_copyable_v<T>);
        // convert all types to array of bytes
        const auto* ptr = reinterpret_cast<const std::byte*>(&value);
        // append the array of bytes to the vector
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    static void append_string_to_buffer(std::vector<std::byte>& buffer, const std::string& value) {
        // uint32_t can represent ~ 4 GB
        const uint32_t size = static_cast<uint32_t>(value.size());
        append_to_buffer(buffer, size);
        const auto* ptr = reinterpret_cast<const std::byte*>(value.data());
        buffer.insert(buffer.end(), ptr, ptr + size);
    }
};

#endif //DUNDERDB_SERIALIZER_H

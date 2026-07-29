//
// Created by Arjun on 29/07/2026.
//

#ifndef DUNDERDB_SERIALIZER_H
#define DUNDERDB_SERIALIZER_H
#include <bitset>
#include <vector>

#include "validated_message.h"

class Serializer {
public:
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
        // uint32_t can represent ~ 4 GB
        const uint32_t size = static_cast<uint32_t>(value.size());
        append_to_buffer(buffer, size);
        const auto* ptr = reinterpret_cast<const std::byte*>(value.data());
        buffer.insert(buffer.end(), ptr, ptr + size);
    }

    static std::vector<std::byte> serialize_message(const ValidatedMessage& message) {
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
            } else {
                const std::string& str = std::get<std::string>(value);
                append_string_to_buffer(buffer, str);
            }
        }

        // Overwrite record placeholder size
        // since the buffer vector uses one byte per element, the size in bytes is simply the number of elements
        const uint32_t record_size = static_cast<uint32_t>(buffer.size());
        std::memcpy(buffer.data(), &record_size, sizeof(record_size));

        return buffer;
    }
};

#endif //DUNDERDB_SERIALIZER_H

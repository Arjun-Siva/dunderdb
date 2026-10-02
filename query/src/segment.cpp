//
// Created by Arjun on 17/09/2026.
//
#include "segment.h"

#include <iostream>

#include "message_serializer.h"
#include "schema_serializer.h"
#include "time_converter.h"

Segment::Segment(const std::vector<std::byte>& segment_binary) {
    // parse the binary blob into schema and messages
    // header_size (uint32) | file_name_size (uint32) | file name | schema binary size (uint32) | schema binary | messages ....

    size_t offset = 0;
    uint32_t header_size;

    if (offset + sizeof(uint32_t) > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&header_size, segment_binary.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + header_size > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    uint32_t filename_size;

    if (offset + sizeof(uint32_t) > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&filename_size, segment_binary.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + filename_size > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    // this filename will be the intended file name of the tmp file after sealing, not seg_service.tmp
    std::string file_name;
    file_name.resize(filename_size);
    std::memcpy(file_name.data(), segment_binary.data() + offset, filename_size);
    offset += filename_size;

    uint32_t schema_binary_size;
    if (offset + sizeof(uint32_t) > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    std::memcpy(&schema_binary_size, segment_binary.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + schema_binary_size > segment_binary.size()) {
        throw std::runtime_error("Corrupted segment file");
    }

    const auto binary_span = std::span<const std::byte>(
        segment_binary.data() + offset,
        schema_binary_size);

    Schema segment_schema = SchemaSerializer::deserialize_bytes_to_schema(binary_span);
    this->schema_ = std::move(segment_schema);

    offset += schema_binary_size;

    // Messages = Message size (uint32) | Message binary | ....
    while (offset < segment_binary.size()) {
        uint32_t message_size;
        if (offset + sizeof(uint32_t) > segment_binary.size()) {
            throw std::runtime_error("Corrupted segment file");
        }

        std::memcpy(&message_size, segment_binary.data() + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (offset + message_size > segment_binary.size()) {
            throw std::runtime_error("Corrupted segment file");
        }

        const auto message_span = std::span(segment_binary.data() + offset, message_size);
        // not catching exceptions since if one message is corrupted, the entire file is corrupted

        ValidatedMessage message = MessageSerializer::deserialize_message(this->schema_, message_span);
        this->messages_vector_.push_back(std::move(message));

        offset += message_size;
    }
}

void Segment::append_query_results(const ParsedSelectQuery& query,
                                   rapidjson::Writer<rapidjson::StringBuffer>& writer,
                                   const bool skip_time_filter) const {
    // convert the timestamps to UTC
    // construct message strings
    const auto columns = this->schema_.get_columns_in_order();

    for (const auto& message : this->messages_vector_) {
        if (!skip_time_filter && (message.timestamp < query.start_time || message.timestamp > query.end_time))
            continue;

        std::string timestamp = TimeConverter::epoch_ms_to_iso(message.timestamp);
        writer.StartObject();
        writer.Key("timestamp");
        writer.String(timestamp.c_str());
        writer.Key("content");
        writer.StartObject();

        for (size_t column_index = 0; column_index < columns.size(); ++column_index) {
            const auto& column = columns[column_index];
            const OptionalValue& val = message.records[column_index];

            writer.Key(column.get_name().c_str());
            if (!val.has_value()) {
                writer.Null();
            }
            else {
                const Value& value = val.value();
                if (std::holds_alternative<double>(value)) {
                    writer.Double(std::get<double>(value));
                }
                else if (std::holds_alternative<std::string>(value)) {
                    writer.String(std::get<std::string>(value).c_str());;
                }
            }
        }
        writer.EndObject();
        writer.EndObject();
    }
}

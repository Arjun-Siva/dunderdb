//
// Created by Arjun on 17/09/2026.
//
#include "segment.h"

#include <iostream>

#include "message_serializer.h"
#include "schema_serializer.h"
#include "segment_header.h"
#include "time_converter.h"

Segment::Segment(const std::vector<std::byte>& segment_binary) {
    // parse the binary blob into schema and messages
    // header_size (uint32) | header_binary | messages ....

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

    const auto header_binary_span = std::span<const std::byte>(
        segment_binary.data() + offset,
        header_size);

    const SegmentHeader header{header_binary_span};

    Schema segment_schema = SchemaSerializer::deserialize_bytes_to_schema(header.get_schema_bytes());
    this->schema_ = std::move(segment_schema);

    offset += header_size;

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

        SequencedMessage message = MessageSerializer::deserialize_message(this->schema_, message_span);
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

    for (const auto& [msg_id, message] : this->messages_vector_) {
        if (!skip_time_filter && (message.timestamp < query.start_time || message.timestamp > query.end_time))
            continue;

        std::string timestamp = TimeConverter::epoch_ms_to_iso(message.timestamp);
        writer.StartObject();
        writer.Key("message_id");
        writer.Uint64(msg_id);
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

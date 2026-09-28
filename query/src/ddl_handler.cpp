//
// Created by Arjun on 26/09/2026.
//
#include <fstream>

#include "ddl_handler.h"
#include "schema_generator.h"
#include "schema_serializer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

std::string DDLHandler::process_ddl_query(const std::string& json_message) const {
    SchemaGenerator schema_gen{json_message};

    // returns error message only if
    if (const auto& error_msg = schema_gen.get_ddl_error_message(this->master_schema_map_);
        error_msg.has_value()) {
        return DDLHandler::form_response("error", error_msg.value());
    }

    // add the new/updated schema to master schema and write to disk
    Schema parsed_schema = schema_gen.get_parsed_schema_object();
    const std::string service_name = schema_gen.get_service_name();

    UnvalidatedMessageType type;

    if (const std::string msg_type = schema_gen.get_ddl_message_type(); msg_type == "new") {
        type = UnvalidatedMessageType::SCHEMA_NEW;
        write_schema_to_disk(parsed_schema, service_name);
        this->master_schema_map_.add_schema(service_name, std::make_shared<Schema>(std::move(parsed_schema)));
        // create new index
        this->master_index_map_.add_index(service_name,
            std::make_shared<ServiceIndex>(service_name, this->indexes_directory_));
    } else if (msg_type == "update") {
        type = UnvalidatedMessageType::SCHEMA_UPDATE;
        write_schema_to_disk(parsed_schema, service_name);
        this->master_schema_map_.add_schema(service_name, std::make_shared<Schema>(std::move(parsed_schema)));
        // no changes in index
    } else {
        type = UnvalidatedMessageType::SCHEMA_DROP;
        delete_schema_from_disk(service_name);
        this->master_schema_map_.delete_schema(service_name);

        // index will be deleted at DiskWriter
    }

    // push to the ingestion queue to be picked up by validator
    UnvalidatedMessage unvalidated_msg {
        .type = type,
        .service = service_name,
        .payload = json_message,
        .timestamp_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
    };

    this->ingestion_queue_.enqueue(std::move(unvalidated_msg));
    return DDLHandler::form_response("ok", "DDL operation successful");
}

void DDLHandler::write_schema_to_disk(Schema& schema, const std::string& service_name) const {
    // create schemas directory if it doesn't exist
    std::filesystem::create_directories(this->schemas_directory_);

    // serialize schema
    std::vector<std::byte> binary = SchemaSerializer::serialize_schema(schema);

    // Create schema file
    std::filesystem::path file_path =
        this->schemas_directory_ / std::string(service_name + ".sch");

    std::ofstream file(
        file_path,
        std::ios::binary | std::ios::trunc
    );

    if (!file.is_open()) {
        throw std::runtime_error(
            "Failed to open file: " + file_path.string()
        );
    }

    file.write(
        reinterpret_cast<const char*>(binary.data()),
        static_cast<std::streamsize>(binary.size())
    );

    if (!file.good()) {
        throw std::runtime_error(
            "Failed while writing file: " + file_path.string()
        );
    }
}

void DDLHandler::delete_schema_from_disk(const std::string& service_name) const {
    const std::filesystem::path file_path = this->schemas_directory_ / std::string(service_name + ".sch");
    std::filesystem::remove(file_path);
}

std::string DDLHandler::form_response(const std::string& status, const std::string& msg) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> rapid_writer(buffer);
    rapid_writer.StartObject();
    rapid_writer.Key("status");
    rapid_writer.String(status.c_str());

    rapid_writer.Key("message");
    rapid_writer.String(msg.c_str());

    rapid_writer.EndObject();
    return buffer.GetString();
}

//
// Created by Arjun on 03/09/2026.
//
#include "rapidjson/document.h"

#include "select_handler.h"

#include <utility>
#include "query_parse_exception.h"
#include "time_converter.h"
#include "file_loader.h"
#include "../include/segment.h"

SelectHandler::SelectHandler(IndexMap &indexMap, LockMap &lockMap, std::filesystem::path services_directory) :
index_map_(indexMap), tmp_file_lock_map_(lockMap), services_directory_(std::move(services_directory)){
}

std::string SelectHandler::get_query_result(const std::string& json_payload) const {
    // parse JSON
    const auto query = parse_select_query(json_payload);
    // index look up on ranges
    // TODO: catch missing service names
    const ServiceIndex& service_index = this->index_map_.get_index(query.service_name);
    std::vector<std::string> file_names = service_index.index_lookup_time_range(query.start_time, query.end_time);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> rapid_writer(buffer);
    rapid_writer.StartObject();

    rapid_writer.Key("rows");
    rapid_writer.StartArray();

    if (file_names.empty()) {
        // close writer and return
        rapid_writer.EndArray();
        rapid_writer.Key("status");
        rapid_writer.String("ok");
        rapid_writer.EndObject();
        std::string response = buffer.GetString();
        return response;
    }

    // check if tmp file is part of the query and read it first
    // tmp file, if present, will always be the last element
    if (file_names.back() == "seg_" + query.service_name + ".tmp") {
        std::vector<std::byte> tmp_file_bytes;
        // acquire read lock
        //query.service_name is the key, not entire file name
        {
            std::shared_lock<std::shared_mutex> lock = tmp_file_lock_map_.get_shared_lock(query.service_name);
            const std::filesystem::path tmp_file_path = this->services_directory_ / query.service_name / file_names.back();
            try {
                tmp_file_bytes = FileLoader::read_file(tmp_file_path);
            } catch (std::runtime_error &e) {
                // continue
            }
        }
        if (!tmp_file_bytes.empty()) {
            Segment tmp_segment{tmp_file_bytes};
            tmp_segment.append_query_results(query, rapid_writer);
        }

        file_names.pop_back();
    }

    // read other files
    // currently locks not needed
    for (size_t i = 0; i < file_names.size(); i++) {
        const auto& file_name = file_names[i];
        const std::filesystem::path file_path = this->services_directory_ / query.service_name / file_name;

        bool skip_time_filter = (i != file_names.size() - 1 && i != 0);
        try {
            if (const auto file_bytes = FileLoader::read_file(file_path); !file_bytes.empty()) {
                Segment file_segment{file_bytes};

                file_segment.append_query_results(query, rapid_writer, skip_time_filter);
            }
        } catch (std::runtime_error &e) {
            // continue
        }
    }

    rapid_writer.EndArray();

    rapid_writer.Key("status");
    rapid_writer.String("ok");
    rapid_writer.EndObject();

    std::string response = buffer.GetString();
    return response;
}


ParsedSelectQuery SelectHandler::parse_select_query(const std::string &json_payload) {
    /* Sample format
     * {
     *  service_name: "sales",
     *  start_time: "2026-09-22T09:31:45.697Z",
     *  end_time: "2026-09-22T09:32:45.697Z"
     * }
     */
    rapidjson::Document doc;
    doc.Parse(json_payload.c_str());

    if (doc.HasParseError())
        throw QueryParseException("Invalid JSON");

    if (!doc.IsObject())
        throw QueryParseException("Query must be a JSON object");

    if (!doc.HasMember("service_name") || !doc["service_name"].IsString())
        throw QueryParseException("Missing or invalid service_name");

    if (!doc.HasMember("start_time") || !doc["start_time"].IsString())
        throw QueryParseException("Missing or invalid start_time");

    if (!doc.HasMember("end_time") || !doc["end_time"].IsString())
        throw QueryParseException("Missing or invalid end_time");

    ParsedSelectQuery query;
    query.service_name = doc["service_name"].GetString();

    try {
        query.start_time = TimeConverter::utc_to_epoch_ms(doc["start_time"].GetString());
    } catch (std::exception &e) {
        throw QueryParseException("Error in parsing start_time UTC timestamp");
    }

    try {
        query.end_time = TimeConverter::utc_to_epoch_ms(doc["end_time"].GetString());
    } catch (std::exception &e) {
        throw QueryParseException("Error in parsing end_time UTC timestamp");
    }

    return query;
}


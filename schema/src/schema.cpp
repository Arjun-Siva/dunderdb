//
// Created by Arjun on 17/06/2026.
//
#include "schema.h"

#include <sstream>
#include <string>
#include <rapidjson/document.h>

void Schema::add_column(const Column& column) {
    std::string column_name = column.get_name();
    this->column_order_.push_back(column_name);
    this->column_map_.emplace(column_name, column);
}

std::optional<RecordsVector> Schema::parse_json(std::string_view json) const {
    rapidjson::Document doc;
    RecordsVector output;

    if (doc.Parse(json.data()).HasParseError()) {
        return std::nullopt;
    }

    if (!doc.IsObject()) {
        return std::nullopt;
    }

    for (const std::string& column : this->column_order_) {
        const Column& col = this->column_map_.at(column);
        const std::string& column_name = col.get_name();
        const bool nullable = col.is_nullable();

        // check if the column is present in the JSON message
        auto column_it = doc.FindMember(column_name.c_str());
        if (column_it == doc.MemberEnd()) {
            if (!nullable) {
                return std::nullopt;
            }
            output.emplace_back(std::nullopt);
            continue;
        }

        // validate the type
        const ColumnType& column_type = col.get_column_type();
        const auto& column_value = column_it->value;

        if (column_type == ColumnType::NUMBER) {
            if (!column_value.IsNumber()) {
                return std::nullopt;
            }
            // NOTICE: currently converting all numeric types to Double

            // if (column_value.IsInt64()) {
            //     output.emplace_back(column_value.GetInt64());
            //     //output_map.emplace(column_name, std::to_string(column_value.GetInt64()));
            // } else {
                output.emplace_back(column_value.GetDouble());
                //output_map.emplace(column_name, std::to_string(column_value.GetDouble()));
            // }
        }

        else {
            if (!column_value.IsString()) {
                return std::nullopt;
            }

            const int max_chars = col.get_max_characters();
            const std::string string_val = column_value.GetString();

            output.emplace_back(string_val.substr(0, max_chars));
        }
    }

    return output;
}

std::string Schema::get_service_name() const {
    return this->service_name_;
}

std::vector<Column> Schema::get_columns_in_order() const {
    std::vector<Column> output;
    output.reserve(this->column_order_.size());

    for (auto& column_name : this->column_order_) {
        output.push_back(this->column_map_.at(column_name));
    }

    return output;
}

std::string Schema::to_string() const {
    std::ostringstream oss;

    oss << "Schema {\n";
    oss << "  service_name: " << service_name_ << "\n";
    oss << "  columns:\n";

    for (const auto& column_name : column_order_) {
        const auto& column = column_map_.at(column_name);

        oss << "    " << column.to_string() << "\n";
    }

    oss << "}";

    return oss.str();
}

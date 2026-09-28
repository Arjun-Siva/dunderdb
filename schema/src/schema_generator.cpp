//
// Created by Arjun on 26/09/2026.
//
#include <rapidjson/document.h>

#include "schema_generator.h"

// Sample message:
// {
//     "change_type": "new / update / drop",
//     "service_name": "sales",
//     "columns": [
//         {
//             "name": "item",
//             "type": "string",
//             "max_characters": 10,
//             "nullable": false
//         },
//         {
//             "name": "price"
//             "type": "number",
//             "max_characters": 10,
//             "nullable": true
//         }
//     ]
// }

std::optional<std::string> is_valid_name(const std::string_view name) {
    // returns error message if invalid, else nullopt
    if (name.empty()) {
        return "empty";
    }

    // First character must be a lowercase letter.
    if (name.front() < 'a' || name.front() > 'z') {
        return "first character must be a lowercase letter";
    }

    // Last character must be a lowercase letter or digit.
    if (name.size() > 1) {
        const char last = name.back();
        if (!((last >= 'a' && last <= 'z') ||
            (last >= '0' && last <= '9'))) {
            return "last character must be a lowercase letter or digit";
        }
    }

    // All characters must be lowercase letters, digits, or '_'.
    for (const char c : name) {
        if (!((c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '_')) {
            return "all characters must be lowercase letters, digits, or '_'";
        }
    }

    return std::nullopt;
}


SchemaGenerator::SchemaGenerator(std::string_view ddl_message) {
    // parse and store columns
    this->valid_schema_ = true;
    rapidjson::Document doc;
    if (doc.Parse(ddl_message.data()).HasParseError()) {
        this->set_error_message_and_validity("Error in parsing message");
        return;
    }

    if (!doc.IsObject()) {
        this->set_error_message_and_validity("Message is not an object");
        return;
    }

    // ------------- change type ---------------
    const auto change_type_iter = doc.FindMember("change_type");
    if (change_type_iter == doc.MemberEnd()) {
        this->set_error_message_and_validity("change_type field is missing");
        return;
    }

    const auto& type = change_type_iter->value;
    if (!type.IsString()) {
        this->set_error_message_and_validity("change_type is not a string");
        return;
    }

    const std::string change_type = type.GetString();
    if (change_type != "new" && change_type != "update" && change_type != "drop") {
        this->set_error_message_and_validity("change_type can be only of types - new, update, and drop");
        return;
    }

    // ------------- service name ---------------
    const auto service_name_iter = doc.FindMember("service_name");
    if (service_name_iter == doc.MemberEnd()) {
        this->set_error_message_and_validity("service_name field is missing");
        return;
    }

    const auto& service = service_name_iter->value;
    if (!service.IsString()) {
        this->set_error_message_and_validity("service_name is not a string");
        return;
    }

    const std::string service_name = service.GetString();
    if (const auto msg = is_valid_name(service_name); msg.has_value()) {
        this->set_error_message_and_validity("Service name - " + msg.value());
        return;
    }

    this->service_name_ = service_name;
    this->ddl_type_ = change_type;

    // -------------- columns -------------------
    const auto columns_iter = doc.FindMember("columns");
    if (columns_iter == doc.MemberEnd()) {
        if (change_type == "new" || change_type == "update") {
            this->set_error_message_and_validity("Columns are missing");
            return;
        }
        // For drop schema, columns are not needed
        return;
    }
    if (change_type == "drop") {
        this->set_error_message_and_validity("For schema DROP, columns are not required");
        return;
    }

    const auto& cols_val = columns_iter->value;
    if (!cols_val.IsArray()) {
        this->set_error_message_and_validity("Columns are not an array");
        return;
    }

    for (const auto& column_obj : cols_val.GetArray()) {
        // ----------- name --------------
        if (!column_obj.IsObject()) {
            this->set_error_message_and_validity("Column value is not an object");
            return;
        }

        const auto name_iter = column_obj.FindMember("name");
        if (name_iter == column_obj.MemberEnd()) {
            this->set_error_message_and_validity("Column name is missing");
            return;
        }

        const auto& name_val = name_iter->value;
        if (!name_val.IsString()) {
            this->set_error_message_and_validity("Column name is not a string");
            return;
        }
        const std::string name = name_val.GetString();

        if (const auto msg = is_valid_name(name); msg.has_value()) {
            this->set_error_message_and_validity("Column name - " + msg.value());
        }

        // ----------- type --------------
        const auto type_iter = column_obj.FindMember("type");
        if (type_iter == column_obj.MemberEnd()) {
            this->set_error_message_and_validity("Column type is missing");
            return;
        }

        const auto& type_val = type_iter->value;
        if (!type_val.IsString()) {
            this->set_error_message_and_validity("Column type is not a string");
            return;
        }
        const std::string col_type = name_val.GetString();

        if (col_type != "string" && col_type != "number") {
            this->set_error_message_and_validity("Column type should be either 'string' or 'number'");
            return;
        }

        // ----------- max chars --------------
        const auto maxchar_iter = column_obj.FindMember("max_characters");
        if (maxchar_iter == column_obj.MemberEnd() && col_type == "string") {
            this->set_error_message_and_validity("Column max_characters is missing");
            return;
        }
        if (col_type == "number") {
            this->set_error_message_and_validity("Column of type 'number' can't have max_characters");
            return;
        }

        const auto& maxchar_val = maxchar_iter->value;
        if (!maxchar_val.IsInt()) {
            this->set_error_message_and_validity("Column max_characters is not an integer");
            return;
        }
        const uint16_t max_characters = maxchar_val.GetInt();

        if (max_characters < 1) {
            this->set_error_message_and_validity("Column max_characters is less than 1");
            return;
        }

        // ----------- nullable --------------
        const auto nullable_iter = column_obj.FindMember("nullable");
        if (nullable_iter == column_obj.MemberEnd()) {
            this->set_error_message_and_validity("Column nullable field is missing");
            return;
        }

        const auto& nullable_val = nullable_iter->value;
        if (!nullable_val.IsBool()) {
            this->set_error_message_and_validity("Column nullable field should be either true or false");
            return;
        }
        const bool nullable = nullable_val.GetBool();

        if (col_type == "string") {
            Column col(name, ColumnType::STRING, max_characters, nullable);
            this->columns_.push_back(std::move(col));
        }
        else {
            Column col(name, ColumnType::NUMBER, nullable);
            this->columns_.push_back(std::move(col));
        }
    }
}

std::optional<std::string> SchemaGenerator::get_ddl_error_message(const SchemaMap& schema_map) {
    if (!this->valid_schema_) {
        return this->error_message_;
    }

    if (this->ddl_type_ == "new" && schema_map.contains(this->service_name_)) {
        this->set_error_message_and_validity("Service already exists");
        return this->error_message_;
    }

    if ((this->ddl_type_ == "drop" || this->ddl_type_ == "update") &&
        !schema_map.contains(this->service_name_)) {
        this->set_error_message_and_validity("Service doesn't exists");
        return this->error_message_;
    }

    return std::nullopt;
}

std::string SchemaGenerator::get_ddl_message_type() const {
    return this->ddl_type_;
}

std::string SchemaGenerator::get_service_name() const {
    return this->service_name_;
}

Schema SchemaGenerator::get_parsed_schema_object() const {
    if (!this->valid_schema_ || this->columns_.empty()) {
        throw std::runtime_error("Schema can't be generated from invalid input message");
    }

    Schema schema(this->service_name_);
    for (auto& column : this->columns_) {
        schema.add_column(column);
    }

    return schema;
}

void SchemaGenerator::set_error_message_and_validity(std::string message) {
    this->valid_schema_ = false;
    this->error_message_ = std::move(message);
}

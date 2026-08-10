//
// Created by Arjun on 17/06/2026.
//

#ifndef DUNDERDB_COLUMN_H
#define DUNDERDB_COLUMN_H
#include <cstdint>
#include <string>

enum class ColumnType {
    NUMBER = 0,
    STRING = 1
};

class Column {
public:
    Column(const std::string &name, ColumnType type, uint16_t max_characters, bool nullable);
    Column(const std::string &name, ColumnType type, bool nullable);
    [[nodiscard]] bool is_valid_type(std::string_view input) const;
    [[nodiscard]] bool is_nullable() const;
    [[nodiscard]] std::string get_name() const;
    [[nodiscard]] uint16_t get_max_characters() const;
    [[nodiscard]] ColumnType get_column_type() const;
    [[nodiscard]] bool get_nullable() const;
private:
    std::string name_;
    ColumnType type_;
    uint16_t max_characters_;
    bool nullable_;
};
#endif //DUNDERDB_COLUMN_H

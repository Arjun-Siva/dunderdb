//
// Created by Arjun on 17/06/2026.
//

#ifndef DUNDERDB_COLUMN_H
#define DUNDERDB_COLUMN_H
#include <string>

enum class ColumnType {
    NUMBER,
    STRING
};

class Column {
public:
    Column(const std::string &name, ColumnType type, int max_characters, bool nullable);
    Column(const std::string &name, ColumnType type, bool nullable);
    bool is_valid_type(std::string_view input) const;
    bool is_nullable() const;
    std::string get_name() const;
    int get_max_characters() const;
    ColumnType get_column_type() const;
private:
    std::string name_;
    ColumnType type_;
    int max_characters_;
    bool nullable_;
};
#endif //DUNDERDB_COLUMN_H

//
// Created by Arjun on 17/06/2026.
//


#include <stdexcept>
#include <cctype>

#include "column.h"

Column::Column(const std::string &name, const ColumnType type, const int max_characters, const bool nullable) {
    if (type != ColumnType::STRING) {
        throw std::invalid_argument("Column::Column()");
    }
    this->name_ = name;
    this->type_ = type;
    this->max_characters_ = max_characters;
    this->nullable_ = nullable;
}

Column::Column(const std::string &name, const ColumnType type, const bool nullable) : max_characters_(0) {
    if (type != ColumnType::NUMBER) {
        throw std::invalid_argument("Column::Column()");
    }
    this->name_ = name;
    this->type_ = type;
    this->nullable_ = nullable;
    this->max_characters_ = 0;
}

// source: https://medium.com/@ryan_forrester_/c-check-if-string-is-number-practical-guide-c7ba6db2febf
bool is_number(const std::string_view s) {
    enum State { START, INT, FRAC, EXP, EXP_NUM };
    State state = START;
    bool has_digits = false;

    for (char c : s) {
        switch (state) {
            case START:
                if (c == '+' || c == '-') state = INT;
                else if (std::isdigit(c)) { state = INT; has_digits = true; }
                else if (c == '.') state = FRAC;
                else return false;
                break;
            case INT:
                if (std::isdigit(c)) has_digits = true;
                else if (c == '.') state = FRAC;
                else if (c == 'e' || c == 'E') state = EXP;
                else return false;
                break;
            case FRAC:
                if (std::isdigit(c)) has_digits = true;
                else if (c == 'e' || c == 'E') state = EXP;
                else return false;
                break;
            case EXP:
                if (c == '+' || c == '-' || std::isdigit(c)) state = EXP_NUM;
                else return false;
                break;
            case EXP_NUM:
                if (!std::isdigit(c)) return false;
                break;
        }
    }

    return has_digits && (state == INT || state == FRAC || state == EXP_NUM);
}

bool Column::is_valid_type(const std::string_view input) const {
    if (this->type_ == ColumnType::STRING) {
        return input.length() <= max_characters_;
    }
    return is_number(input);
}

std::string Column::get_name() const {
    return this->name_;
}

bool Column::is_nullable() const {
    return this->nullable_;
}

int Column::get_max_characters() const {
    return this->max_characters_;
}

ColumnType Column::get_column_type() const {
    return this->type_;
}



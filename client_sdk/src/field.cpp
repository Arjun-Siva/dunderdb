#include "dunderdb/field.h"

namespace dunderdb {

    Field::Field(std::string name, Value value)
        : name_(std::move(name)),
          value_(std::move(value)) {}

    const std::string& Field::name() const {
        return name_;
    }

    bool Field::is_null() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

} // namespace dunderdb
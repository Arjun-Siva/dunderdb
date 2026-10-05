#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace dunderdb {

    using Value = std::variant<
        double,
        std::string,
        std::nullptr_t
    >;

    class Field {
    public:
        Field(std::string name, Value value);

        const std::string& name() const;

        template <typename T>
        const T& as() const;

        bool is_null() const;

    private:
        std::string name_;
        Value value_;
    };

    template <typename T>
    const T& Field::as() const {
        return std::get<T>(value_);
    }
} // namespace dunderdb
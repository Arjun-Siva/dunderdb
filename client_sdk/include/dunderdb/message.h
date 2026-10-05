#pragma once

#include "field.h"

#include <string>
#include <vector>

namespace dunderdb {

    class Message {
    public:
        Message() = default;

        Message(std::initializer_list<Field> fields);

        void add_field(Field field);

        const std::vector<Field>& fields() const;

        const Field* get_field(const std::string& name) const;

    private:
        std::vector<Field> fields_;
    };

} // namespace dunderdb
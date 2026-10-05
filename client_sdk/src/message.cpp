#include "dunderdb/message.h"

namespace dunderdb
{
    Message::Message(std::initializer_list<Field> fields)
        : fields_(fields) {}

    void Message::add_field(Field field) {
        fields_.push_back(std::move(field));
    }

    const std::vector<Field>& Message::fields() const {
        return fields_;
    }

    const Field* Message::get_field(const std::string& name) const {
        for (const auto& field : fields_) {
            if (field.name() == name) {
                return &field;
            }
        }

        return nullptr;
    }
} // namespace dunderdb

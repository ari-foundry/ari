#include "std_rc_semantics.hpp"

namespace ari {

bool is_std_rc_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           (type.name == "std::rc::Rc" ||
            type.name == "std::rc::Arc" ||
            type.name == "std::rc::Weak");
}

std::optional<std::size_t> std_rc_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_rc_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 1 || type.field_types.size() != 1) return std::nullopt;
    if (type.field_names[0] != "control") return std::nullopt;
    if (type.field_types[0].qualifier != TypeQualifier::Ptr) return std::nullopt;
    return 0;
}

} // namespace ari

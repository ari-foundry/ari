#include "std_cell_semantics.hpp"

namespace ari {

bool is_std_cell_zone_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           (type.name == "std::cell::OnceCell" ||
            type.name == "std::cell::Lazy");
}

std::optional<std::size_t> std_cell_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_cell_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.empty() || type.field_types.empty()) return std::nullopt;
    if (type.name == "std::cell::OnceCell") {
        if (type.field_names[0] != "data") return std::nullopt;
        if (type.field_types[0].qualifier != TypeQualifier::Ptr) return std::nullopt;
        return 0;
    }
    if (type.name == "std::cell::Lazy") {
        if (type.field_names[0] != "cell") return std::nullopt;
        return 0;
    }
    return std::nullopt;
}

} // namespace ari

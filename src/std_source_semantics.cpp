#include "std_source_semantics.hpp"

#include "type_semantics.hpp"

#include <string>

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

} // namespace

bool is_std_source_line_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::source::LineMap";
}

std::optional<std::size_t> std_source_line_map_zone_handle_source_field_index(
    const IrType& type) {
    if (!is_std_source_line_map_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 1;
    if (type.field_names.size() != 3 || type.field_types.size() != 3) {
        return std::nullopt;
    }

    bool has_file = false;
    bool has_lines = false;
    std::optional<std::size_t> starts_index;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "file") {
            if (field_type.qualifier != TypeQualifier::Value ||
                field_type.primitive != IrPrimitiveKind::Struct ||
                field_type.name != "std::source::SourceFile") {
                return std::nullopt;
            }
            has_file = true;
        } else if (name == "starts") {
            if (field_type.qualifier != TypeQualifier::Ptr ||
                field_type.primitive != IrPrimitiveKind::I64) {
                return std::nullopt;
            }
            starts_index = i;
        } else if (name == "lines") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_lines = true;
        } else {
            return std::nullopt;
        }
    }

    if (!has_file || !starts_index || !has_lines) return std::nullopt;
    return starts_index;
}

} // namespace ari

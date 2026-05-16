#include "format_semantics.hpp"

#include "type_semantics.hpp"

#include <utility>

namespace ari {

std::optional<FormatInAppendTarget> builtin_format_in_append_target_from_type(const IrType& type) {
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::String) {
        return FormatInAppendTarget{FormatInAppendKind::String, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::Bool) {
        return FormatInAppendTarget{FormatInAppendKind::Bool, {}};
    }
    if (is_value_integer_type(type)) {
        return FormatInAppendTarget{FormatInAppendKind::I64, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::F32) {
        return FormatInAppendTarget{FormatInAppendKind::F32, {}};
    }
    if (type.qualifier == TypeQualifier::Value && type.primitive == IrPrimitiveKind::F64) {
        return FormatInAppendTarget{FormatInAppendKind::F64, {}};
    }
    return std::nullopt;
}

FormatInAppendTarget format_in_display_append_target(std::string trait_name) {
    return FormatInAppendTarget{FormatInAppendKind::Display, std::move(trait_name)};
}

bool format_in_append_target_is_float(const FormatInAppendTarget& target) {
    return target.kind == FormatInAppendKind::F32 ||
           target.kind == FormatInAppendKind::F64;
}

bool format_in_append_target_is_display(const FormatInAppendTarget& target) {
    return target.kind == FormatInAppendKind::Display;
}

const char* format_in_builtin_append_method_name(const FormatInAppendTarget& target) {
    switch (target.kind) {
        case FormatInAppendKind::String: return "append_string_in";
        case FormatInAppendKind::I64: return "append_i64_in";
        case FormatInAppendKind::Bool: return "append_bool_in";
        case FormatInAppendKind::F32: return "append_f32_in";
        case FormatInAppendKind::F64: return "append_f64_in";
        case FormatInAppendKind::Display: break;
    }
    return "append_i64_in";
}

std::string unsupported_format_in_value_message(const IrType& type) {
    return "format_in! currently supports string, integer, bool, f32, f64, and Display values, got " +
           type_name(type);
}

} // namespace ari

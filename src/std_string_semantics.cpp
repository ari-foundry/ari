#include "std_string_semantics.hpp"

#include "type_semantics.hpp"

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

bool is_u8_ptr_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ptr &&
           type.primitive == IrPrimitiveKind::U8;
}

IrType value_qualified_string_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

} // namespace

std::optional<std::size_t> std_string_raw_handle_data_field_index(const IrType& type) {
    if (!is_std_string_raw_handle_type(type)) {
        return std::nullopt;
    }
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 3 || type.field_types.size() != 3) return std::nullopt;

    std::optional<std::size_t> data_index;
    bool has_len = false;
    bool has_capacity = false;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "data") {
            if (!is_u8_ptr_type(field_type)) return std::nullopt;
            data_index = i;
        } else if (name == "len") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_len = true;
        } else if (name == "capacity") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_capacity = true;
        } else {
            return std::nullopt;
        }
    }

    if (!data_index || !has_len || !has_capacity) return std::nullopt;
    return data_index;
}

bool is_std_string_raw_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::string::RawString";
}

bool is_std_string_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::string::String";
}

bool is_std_string_zone_handle_type(const IrType& type) {
    return is_std_string_raw_handle_type(type) || is_std_string_handle_type(type);
}

std::optional<std::size_t> std_string_zone_handle_source_field_index(const IrType& type) {
    if (is_std_string_raw_handle_type(type)) {
        return std_string_raw_handle_data_field_index(type);
    }
    if (!is_std_string_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 1 || type.field_types.size() != 1) return std::nullopt;
    if (type.field_names[0] != "raw") return std::nullopt;
    if (!is_std_string_raw_handle_type(type.field_types[0])) return std::nullopt;
    return 0;
}

bool std_string_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_std_string_handle_type(value_qualified_string_type(call.args[0]->type));
}

bool std_string_extern_builtin_allows_zone_pointer_argument(const std::string& function_name,
                                                           std::size_t arg_index) {
    return arg_index == 0 &&
           (function_name == "string::copy_to" ||
            function_name == "std::string::copy_to");
}

} // namespace ari

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

std::optional<std::vector<std::size_t>> std_string_zone_handle_data_field_path_indices(const IrType& type) {
    if (is_std_string_raw_handle_type(type)) {
        std::optional<std::size_t> data_index = std_string_raw_handle_data_field_index(type);
        if (!data_index) return std::nullopt;
        return std::vector<std::size_t>{*data_index};
    }
    if (!is_std_string_handle_type(type)) return std::nullopt;

    std::optional<std::size_t> raw_index = std_string_zone_handle_source_field_index(type);
    if (!raw_index || *raw_index >= type.field_types.size()) return std::nullopt;
    std::optional<std::size_t> data_index =
        std_string_raw_handle_data_field_index(type.field_types[*raw_index]);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*raw_index, *data_index};
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

bool std_string_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "reserve" ||
           method_name == "reserve_extra" ||
           method_name == "push_in" ||
           method_name == "append_string_in" ||
           method_name == "append_i64_in" ||
           method_name == "append_u64_in" ||
           method_name == "append_bool_in" ||
           method_name == "append_f32_in" ||
           method_name == "append_f64_in" ||
           method_name == "insert_in" ||
           method_name == "extend_from_slice_in" ||
           method_name == "resize_in";
}

std::optional<std::string> std_string_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdStringZoneSourceLookup& receiver_zone_source,
    const StdStringZoneSourceLookup& argument_zone_source) {
    if (!std_string_method_requires_same_zone_argument(method_name) || args.size() < 2) return std::nullopt;
    if (!is_std_string_handle_type(value_qualified_string_type(receiver_type))) return std::nullopt;

    std::string string_source;
    if (!receiver_zone_source(*args[0], string_source)) {
        return "std::string::String." + method_name + " receiver must come from a tracked zone allocation";
    }

    std::string zone_source;
    if (!argument_zone_source(*args[1], zone_source)) {
        return "std::string::String." + method_name + " requires an explicit zone borrow argument";
    }

    if (string_source != zone_source) {
        return "std::string::String." + method_name + " zone argument must match the string allocation zone";
    }

    return std::nullopt;
}

} // namespace ari

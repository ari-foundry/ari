#include "std_vec_semantics.hpp"

#include "type_semantics.hpp"

#include <string>

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

IrType value_qualified_vec_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

} // namespace

std::optional<std::size_t> std_vec_raw_handle_data_field_index(const IrType& type) {
    if (!is_std_vec_raw_handle_type(type)) {
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
            if (field_type.qualifier != TypeQualifier::Ptr) return std::nullopt;
            if (!type.args.empty()) {
                IrType expected_data = type.args[0];
                expected_data.qualifier = TypeQualifier::Ptr;
                if (!same_type(field_type, expected_data)) return std::nullopt;
            }
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

bool is_std_vec_raw_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::vec::RawVec";
}

bool is_std_vec_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::vec::Vec";
}

bool is_std_vec_zone_handle_type(const IrType& type) {
    return is_std_vec_raw_handle_type(type) || is_std_vec_handle_type(type);
}

std::optional<std::size_t> std_vec_zone_handle_source_field_index(const IrType& type) {
    if (is_std_vec_raw_handle_type(type)) {
        return std_vec_raw_handle_data_field_index(type);
    }
    if (!is_std_vec_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 1 || type.field_types.size() != 1) return std::nullopt;
    if (type.field_names[0] != "raw") return std::nullopt;
    if (!is_std_vec_raw_handle_type(type.field_types[0])) return std::nullopt;
    return 0;
}

std::optional<std::vector<std::size_t>> std_vec_zone_handle_data_field_path_indices(const IrType& type) {
    if (is_std_vec_raw_handle_type(type)) {
        std::optional<std::size_t> data_index = std_vec_raw_handle_data_field_index(type);
        if (!data_index) return std::nullopt;
        return std::vector<std::size_t>{*data_index};
    }
    if (!is_std_vec_handle_type(type)) return std::nullopt;

    std::optional<std::size_t> raw_index = std_vec_zone_handle_source_field_index(type);
    if (!raw_index || *raw_index >= type.field_types.size()) return std::nullopt;
    std::optional<std::size_t> data_index =
        std_vec_raw_handle_data_field_index(type.field_types[*raw_index]);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*raw_index, *data_index};
}

bool std_vec_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "reserve" ||
           method_name == "reserve_extra" ||
           method_name == "push_in" ||
           method_name == "insert_in" ||
           method_name == "extend_from_slice_in" ||
           method_name == "resize_in";
}

bool std_vec_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_std_vec_handle_type(value_qualified_vec_type(call.args[0]->type));
}

} // namespace ari

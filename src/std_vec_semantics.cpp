#include "std_vec_semantics.hpp"

#include "type_semantics.hpp"

#include <string>

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

bool is_zone_metadata_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::zone::ZoneMetadata";
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

static std::optional<std::size_t> std_vec_iter_handle_data_field_index(const IrType& type) {
    if (!is_std_vec_iter_handle_type(type)) {
        return std::nullopt;
    }
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 3 || type.field_types.size() != 3) return std::nullopt;

    std::optional<std::size_t> data_index;
    bool has_len = false;
    bool has_index = false;
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
        } else if (name == "index") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_index = true;
        } else {
            return std::nullopt;
        }
    }

    if (!data_index || !has_len || !has_index) return std::nullopt;
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

bool is_std_vec_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::vec::Iter";
}

bool is_std_vec_zone_handle_type(const IrType& type) {
    return is_std_vec_raw_handle_type(type) ||
           is_std_vec_handle_type(type) ||
           is_std_vec_iter_handle_type(type);
}

std::optional<std::size_t> std_vec_zone_handle_source_field_index(const IrType& type) {
    if (is_std_vec_raw_handle_type(type)) {
        return std_vec_raw_handle_data_field_index(type);
    }
    if (is_std_vec_iter_handle_type(type)) {
        return std_vec_iter_handle_data_field_index(type);
    }
    if (!is_std_vec_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 1;
    if (type.field_names.size() != 2 || type.field_types.size() != 2) return std::nullopt;

    std::optional<std::size_t> raw_index;
    std::optional<std::size_t> metadata_index;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "raw") {
            if (!is_std_vec_raw_handle_type(field_type)) return std::nullopt;
            raw_index = i;
        } else if (name == "metadata") {
            if (!is_zone_metadata_type(field_type)) return std::nullopt;
            metadata_index = i;
        } else {
            return std::nullopt;
        }
    }
    if (!raw_index || !metadata_index) return std::nullopt;
    return metadata_index;
}

std::optional<std::vector<std::size_t>> std_vec_zone_handle_data_field_path_indices(const IrType& type) {
    if (is_std_vec_raw_handle_type(type)) {
        std::optional<std::size_t> data_index = std_vec_raw_handle_data_field_index(type);
        if (!data_index) return std::nullopt;
        return std::vector<std::size_t>{*data_index};
    }
    if (is_std_vec_iter_handle_type(type)) {
        std::optional<std::size_t> data_index = std_vec_iter_handle_data_field_index(type);
        if (!data_index) return std::nullopt;
        return std::vector<std::size_t>{*data_index};
    }
    if (!is_std_vec_handle_type(type)) return std::nullopt;

    std::optional<std::size_t> raw_index;
    if (type.field_names.empty() && type.field_types.empty()) {
        raw_index = 0;
    } else {
        for (std::size_t i = 0; i < type.field_names.size(); ++i) {
            if (type.field_names[i] == "raw") {
                raw_index = i;
                break;
            }
        }
    }
    if (!raw_index || *raw_index >= type.field_types.size()) return std::nullopt;
    std::optional<std::size_t> data_index =
        std_vec_raw_handle_data_field_index(type.field_types[*raw_index]);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*raw_index, *data_index};
}

std::vector<std::vector<std::size_t>> std_vec_zone_handle_storage_field_path_indices(const IrType& type) {
    if (!std_vec_zone_handle_source_field_index(type)) return {};
    if (is_std_vec_raw_handle_type(type)) {
        std::optional<std::size_t> data_index = std_vec_raw_handle_data_field_index(type);
        if (!data_index) return {};
        return {{*data_index}};
    }
    if (is_std_vec_iter_handle_type(type)) {
        std::optional<std::size_t> data_index = std_vec_iter_handle_data_field_index(type);
        if (!data_index) return {};
        return {{*data_index}};
    }
    if (!is_std_vec_handle_type(type)) return {};
    if (type.field_names.empty() && type.field_types.empty()) return {{0}, {1}};

    std::optional<std::size_t> raw_index;
    std::optional<std::size_t> metadata_index;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        if (type.field_names[i] == "raw") raw_index = i;
        if (type.field_names[i] == "metadata") metadata_index = i;
    }

    std::vector<std::vector<std::size_t>> paths;
    if (raw_index) paths.push_back({*raw_index});
    if (metadata_index) paths.push_back({*metadata_index});
    return paths;
}

bool std_vec_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "reserve_in" ||
           method_name == "reserve_extra_in" ||
           method_name == "push_in" ||
           method_name == "insert_in" ||
           method_name == "extend_from_slice_in" ||
           method_name == "resize_in" ||
           method_name == "split_off";
}

std::optional<StdVecImplicitZoneMethod> std_vec_implicit_zone_method_for_call(
    const std::string& method_name,
    std::size_t user_arg_count) {
    if (method_name == "split_off" && user_arg_count == 1) {
        return StdVecImplicitZoneMethod{"split_off", false};
    }
    return std::nullopt;
}

bool std_vec_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_std_vec_handle_type(value_qualified_vec_type(call.args[0]->type));
}

bool std_vec_result_preserves_receiver_zone(const IrExpr& call) {
    if (call.kind != IrExprKind::Call || call.args.empty()) return false;
    if (!is_std_vec_handle_type(value_qualified_vec_type(call.args[0]->type))) return false;
    IrType result_type = value_qualified_vec_type(call.type);
    return call.type.qualifier == TypeQualifier::Ptr ||
           is_std_vec_iter_handle_type(result_type);
}

std::optional<std::string> std_vec_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdVecZoneSourceLookup& receiver_zone_source,
    const StdVecZoneSourceLookup& argument_zone_source) {
    if (!std_vec_method_requires_same_zone_argument(method_name) || args.size() < 2) return std::nullopt;
    if (!is_std_vec_handle_type(value_qualified_vec_type(receiver_type))) return std::nullopt;

    std::string vec_source;
    if (!receiver_zone_source(*args[0], vec_source)) {
        return "std::vec::Vec." + method_name + " receiver must come from a tracked zone allocation";
    }

    std::string zone_source;
    if (!argument_zone_source(*args[1], zone_source)) {
        return "std::vec::Vec." + method_name + " requires an explicit zone borrow argument";
    }

    if (vec_source != zone_source) {
        return "std::vec::Vec." + method_name + " zone argument must match the vector allocation zone";
    }

    return std::nullopt;
}

} // namespace ari

#include "std_collections_semantics.hpp"

#include "type_semantics.hpp"

namespace ari {

namespace {

bool is_i64_value_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

IrType value_qualified_set_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

} // namespace

bool is_std_collections_set_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::Set";
}

bool is_std_collections_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::Iter";
}

bool is_std_collections_zone_handle_type(const IrType& type) {
    return is_std_collections_set_handle_type(type) ||
           is_std_collections_iter_handle_type(type);
}

std::optional<std::size_t> std_collections_set_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_collections_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;

    std::optional<std::size_t> data_index;
    bool has_len = false;
    bool has_capacity = false;
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
        } else if (name == "capacity") {
            if (!is_std_collections_set_handle_type(type)) return std::nullopt;
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_capacity = true;
        } else if (name == "index") {
            if (!is_std_collections_iter_handle_type(type)) return std::nullopt;
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_index = true;
        } else {
            return std::nullopt;
        }
    }

    if (!data_index || !has_len) return std::nullopt;
    if (is_std_collections_set_handle_type(type) && !has_capacity) return std::nullopt;
    if (is_std_collections_iter_handle_type(type) && !has_index) return std::nullopt;
    return data_index;
}

std::optional<std::vector<std::size_t>> std_collections_set_zone_handle_data_field_path_indices(const IrType& type) {
    std::optional<std::size_t> data_index = std_collections_set_zone_handle_source_field_index(type);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*data_index};
}

bool std_collections_set_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "grow_to_capacity" ||
           method_name == "ensure_spare_capacity" ||
           method_name == "reserve" ||
           method_name == "reserve_extra" ||
           method_name == "insert";
}

bool std_collections_result_preserves_receiver_zone(const IrExpr& call) {
    if (call.kind != IrExprKind::Call || call.args.empty()) return false;
    if (!is_std_collections_set_handle_type(value_qualified_set_type(call.args[0]->type))) return false;
    IrType result_type = value_qualified_set_type(call.type);
    return is_std_collections_iter_handle_type(result_type);
}

std::optional<std::string> std_collections_set_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdCollectionsZoneSourceLookup& receiver_zone_source,
    const StdCollectionsZoneSourceLookup& argument_zone_source) {
    if (!std_collections_set_method_requires_same_zone_argument(method_name) || args.size() < 2) {
        return std::nullopt;
    }
    if (!is_std_collections_set_handle_type(value_qualified_set_type(receiver_type))) return std::nullopt;

    std::string set_source;
    if (!receiver_zone_source(*args[0], set_source)) {
        return "std::collections::Set." + method_name + " receiver must come from a tracked zone allocation";
    }

    std::string zone_source;
    if (!argument_zone_source(*args[1], zone_source)) {
        return "std::collections::Set." + method_name + " requires an explicit zone borrow argument";
    }

    if (set_source != zone_source) {
        return "std::collections::Set." + method_name + " zone argument must match the set allocation zone";
    }

    return std::nullopt;
}

} // namespace ari

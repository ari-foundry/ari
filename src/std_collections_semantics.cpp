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

bool is_i64_ptr_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Ptr &&
           type.primitive == IrPrimitiveKind::I64;
}

bool ptr_field_matches_type_arg(const IrType& field_type, const IrType& type, std::size_t arg_index) {
    if (field_type.qualifier != TypeQualifier::Ptr) return false;
    if (arg_index >= type.args.size()) return true;
    IrType expected = type.args[arg_index];
    expected.qualifier = TypeQualifier::Ptr;
    return same_type(field_type, expected);
}

std::string collection_type_display_name(const IrType& type) {
    if (type.name == "std::collections::HashMap") return "std::collections::HashMap";
    if (type.name == "std::collections::HashSet") return "std::collections::HashSet";
    if (type.name == "std::collections::TreeMap") return "std::collections::TreeMap";
    if (type.name == "std::collections::TreeSet") return "std::collections::TreeSet";
    if (type.name == "std::collections::Iter") return "std::collections::Iter";
    return "std::collections::Set";
}

std::string collection_allocation_noun(const IrType& type) {
    if (type.name == "std::collections::HashMap") return "hash map";
    if (type.name == "std::collections::HashSet") return "hash set";
    if (type.name == "std::collections::TreeMap") return "tree map";
    if (type.name == "std::collections::TreeSet") return "tree set";
    return "set";
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

static bool is_std_collections_hash_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashMap";
}

static bool is_std_collections_hash_set_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashSet";
}

static bool is_std_collections_tree_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeMap";
}

static bool is_std_collections_tree_set_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeSet";
}

bool is_std_collections_mutable_handle_type(const IrType& type) {
    return is_std_collections_set_handle_type(type) ||
           is_std_collections_hash_map_handle_type(type) ||
           is_std_collections_hash_set_handle_type(type) ||
           is_std_collections_tree_map_handle_type(type) ||
           is_std_collections_tree_set_handle_type(type);
}

bool is_std_collections_zone_handle_type(const IrType& type) {
    return is_std_collections_set_handle_type(type) ||
           is_std_collections_iter_handle_type(type) ||
           is_std_collections_hash_map_handle_type(type) ||
           is_std_collections_hash_set_handle_type(type) ||
           is_std_collections_tree_map_handle_type(type) ||
           is_std_collections_tree_set_handle_type(type);
}

std::optional<std::size_t> std_collections_set_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_collections_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) {
        if (is_std_collections_hash_map_handle_type(type) ||
            is_std_collections_hash_set_handle_type(type) ||
            is_std_collections_tree_map_handle_type(type) ||
            is_std_collections_tree_set_handle_type(type)) {
            return 1;
        }
        return 0;
    }

    std::optional<std::size_t> data_index;
    std::optional<std::size_t> keys_index;
    bool has_len = false;
    bool has_capacity = false;
    bool has_index = false;
    bool has_hash_or_less = false;
    bool has_values = false;
    bool has_states = false;
    bool has_left = false;
    bool has_right = false;
    bool has_parent = false;
    bool has_colors = false;
    bool has_root = false;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        const IrType& field_type = type.field_types[i];
        if (name == "hash" || name == "less") {
            has_hash_or_less = true;
        } else if (name == "data") {
            if (!ptr_field_matches_type_arg(field_type, type, 0)) return std::nullopt;
            data_index = i;
        } else if (name == "keys") {
            if (!ptr_field_matches_type_arg(field_type, type, 0)) return std::nullopt;
            keys_index = i;
        } else if (name == "values") {
            if (!ptr_field_matches_type_arg(field_type, type, 1)) return std::nullopt;
            has_values = true;
        } else if (name == "states") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_states = true;
        } else if (name == "left") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_left = true;
        } else if (name == "right") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_right = true;
        } else if (name == "parent") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_parent = true;
        } else if (name == "colors") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_colors = true;
        } else if (name == "len") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_len = true;
        } else if (name == "capacity") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_capacity = true;
        } else if (name == "index") {
            if (!is_std_collections_iter_handle_type(type)) return std::nullopt;
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_index = true;
        } else if (name == "root") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_root = true;
        } else {
            return std::nullopt;
        }
    }

    if (is_std_collections_set_handle_type(type)) {
        if (!data_index || !has_len || !has_capacity) return std::nullopt;
        return data_index;
    }
    if (is_std_collections_iter_handle_type(type)) {
        if (!data_index || !has_len || !has_index) return std::nullopt;
        return data_index;
    }
    if (is_std_collections_hash_map_handle_type(type)) {
        if (!has_hash_or_less || !keys_index || !has_values || !has_states || !has_len || !has_capacity) {
            return std::nullopt;
        }
        return keys_index;
    }
    if (is_std_collections_hash_set_handle_type(type)) {
        if (!has_hash_or_less || !data_index || !has_states || !has_len || !has_capacity) {
            return std::nullopt;
        }
        return data_index;
    }
    if (is_std_collections_tree_map_handle_type(type)) {
        if (!has_hash_or_less || !keys_index || !has_values || !has_left || !has_right ||
            !has_parent || !has_colors || !has_root || !has_len || !has_capacity) {
            return std::nullopt;
        }
        return keys_index;
    }
    if (is_std_collections_tree_set_handle_type(type)) {
        if (!has_hash_or_less || !data_index || !has_left || !has_right ||
            !has_parent || !has_colors || !has_root || !has_len || !has_capacity) {
            return std::nullopt;
        }
        return data_index;
    }
    return data_index;
}

std::optional<std::vector<std::size_t>> std_collections_set_zone_handle_data_field_path_indices(const IrType& type) {
    std::optional<std::size_t> data_index = std_collections_set_zone_handle_source_field_index(type);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*data_index};
}

std::vector<std::vector<std::size_t>> std_collections_zone_handle_storage_field_path_indices(const IrType& type) {
    if (!std_collections_set_zone_handle_source_field_index(type)) return {};
    if (type.field_names.empty() && type.field_types.empty()) {
        if (is_std_collections_hash_map_handle_type(type)) return {{1}, {2}, {3}};
        if (is_std_collections_hash_set_handle_type(type)) return {{1}, {2}};
        if (is_std_collections_tree_map_handle_type(type)) return {{1}, {2}, {3}, {4}, {5}, {6}};
        if (is_std_collections_tree_set_handle_type(type)) return {{1}, {2}, {3}, {4}, {5}};
        return {{0}};
    }

    std::vector<std::vector<std::size_t>> paths;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        if (name == "data" || name == "keys" || name == "values" || name == "states" ||
            name == "left" || name == "right" || name == "parent" || name == "colors") {
            paths.push_back(std::vector<std::size_t>{i});
        }
    }
    return paths;
}

bool std_collections_set_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "grow_to_capacity" ||
           method_name == "ensure_spare_capacity" ||
           method_name == "hash_map_grow_to_capacity" ||
           method_name == "hash_map_ensure_spare_capacity" ||
           method_name == "hash_set_grow_to_capacity" ||
           method_name == "hash_set_ensure_spare_capacity" ||
           method_name == "tree_map_grow_to_capacity" ||
           method_name == "tree_set_grow_to_capacity" ||
           method_name == "reserve" ||
           method_name == "reserve_extra" ||
           method_name == "replace" ||
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
    IrType receiver_value_type = value_qualified_set_type(receiver_type);
    if (!is_std_collections_mutable_handle_type(receiver_value_type)) return std::nullopt;

    std::string type_name = collection_type_display_name(receiver_value_type);

    std::string receiver_source;
    if (!receiver_zone_source(*args[0], receiver_source)) {
        return type_name + "." + method_name + " receiver must come from a tracked zone allocation";
    }

    std::string zone_source;
    if (!argument_zone_source(*args[1], zone_source)) {
        return type_name + "." + method_name + " requires an explicit zone borrow argument";
    }

    if (receiver_source != zone_source) {
        return type_name + "." + method_name +
               " zone argument must match the " +
               collection_allocation_noun(receiver_value_type) +
               " allocation zone";
    }

    return std::nullopt;
}

} // namespace ari

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
    if (type.name == "std::collections::HashMapEntry") return "std::collections::HashMapEntry";
    if (type.name == "std::collections::HashMapKeys") return "std::collections::HashMapKeys";
    if (type.name == "std::collections::HashMapValues") return "std::collections::HashMapValues";
    if (type.name == "std::collections::HashSet") return "std::collections::HashSet";
    if (type.name == "std::collections::HashSetIter") return "std::collections::HashSetIter";
    if (type.name == "std::collections::TreeMap") return "std::collections::TreeMap";
    if (type.name == "std::collections::TreeMapEntry") return "std::collections::TreeMapEntry";
    if (type.name == "std::collections::TreeMapKeys") return "std::collections::TreeMapKeys";
    if (type.name == "std::collections::TreeMapValues") return "std::collections::TreeMapValues";
    if (type.name == "std::collections::TreeSet") return "std::collections::TreeSet";
    if (type.name == "std::collections::TreeSetIter") return "std::collections::TreeSetIter";
    if (type.name == "std::collections::Deque") return "std::collections::Deque";
    if (type.name == "std::collections::DequeIter") return "std::collections::DequeIter";
    if (type.name == "std::collections::RingBuffer") return "std::collections::RingBuffer";
    if (type.name == "std::collections::RingBufferIter") return "std::collections::RingBufferIter";
    if (type.name == "std::collections::LinkedList") return "std::collections::LinkedList";
    if (type.name == "std::collections::LinkedListIter") return "std::collections::LinkedListIter";
    if (type.name == "std::collections::BinaryHeap") return "std::collections::BinaryHeap";
    if (type.name == "std::collections::PriorityQueue") return "std::collections::PriorityQueue";
    if (type.name == "std::collections::Iter") return "std::collections::Iter";
    return "std::collections::Set";
}

std::string collection_allocation_noun(const IrType& type) {
    if (type.name == "std::collections::HashMap") return "hash map";
    if (type.name == "std::collections::HashSet") return "hash set";
    if (type.name == "std::collections::TreeMap") return "tree map";
    if (type.name == "std::collections::TreeSet") return "tree set";
    if (type.name == "std::collections::Deque") return "deque";
    if (type.name == "std::collections::RingBuffer") return "ring buffer";
    if (type.name == "std::collections::LinkedList") return "linked list";
    if (type.name == "std::collections::BinaryHeap") return "binary heap";
    if (type.name == "std::collections::PriorityQueue") return "priority queue";
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
           (type.name == "std::collections::Iter" ||
            type.name == "std::collections::HashMapKeys" ||
            type.name == "std::collections::HashMapValues" ||
            type.name == "std::collections::HashSetIter" ||
            type.name == "std::collections::TreeMapKeys" ||
            type.name == "std::collections::TreeMapValues" ||
            type.name == "std::collections::TreeSetIter" ||
            type.name == "std::collections::DequeIter" ||
            type.name == "std::collections::RingBufferIter" ||
            type.name == "std::collections::LinkedListIter");
}

static bool is_std_collections_hash_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashMap";
}

static bool is_std_collections_hash_map_entry_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashMapEntry";
}

static bool is_std_collections_hash_map_keys_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashMapKeys";
}

static bool is_std_collections_hash_map_values_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashMapValues";
}

static bool is_std_collections_hash_set_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashSet";
}

static bool is_std_collections_hash_set_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::HashSetIter";
}

static bool is_std_collections_tree_map_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeMap";
}

static bool is_std_collections_tree_map_entry_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeMapEntry";
}

static bool is_std_collections_tree_map_keys_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeMapKeys";
}

static bool is_std_collections_tree_map_values_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeMapValues";
}

static bool is_std_collections_tree_set_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeSet";
}

static bool is_std_collections_tree_set_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::TreeSetIter";
}

static bool is_std_collections_deque_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::Deque";
}

static bool is_std_collections_deque_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::DequeIter";
}

static bool is_std_collections_ring_buffer_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::RingBuffer";
}

static bool is_std_collections_ring_buffer_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::RingBufferIter";
}

static bool is_std_collections_linked_list_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::LinkedList";
}

static bool is_std_collections_linked_list_iter_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::LinkedListIter";
}

static bool is_std_collections_binary_heap_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::BinaryHeap";
}

static bool is_std_collections_priority_queue_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::collections::PriorityQueue";
}

bool is_std_collections_map_update_entry_handle_type(const IrType& type) {
    return is_std_collections_hash_map_entry_handle_type(type) ||
           is_std_collections_tree_map_entry_handle_type(type);
}

bool is_std_collections_mutable_handle_type(const IrType& type) {
    return is_std_collections_set_handle_type(type) ||
           is_std_collections_hash_map_handle_type(type) ||
           is_std_collections_hash_set_handle_type(type) ||
           is_std_collections_tree_map_handle_type(type) ||
           is_std_collections_tree_set_handle_type(type) ||
           is_std_collections_deque_handle_type(type) ||
           is_std_collections_ring_buffer_handle_type(type) ||
           is_std_collections_linked_list_handle_type(type) ||
           is_std_collections_binary_heap_handle_type(type) ||
           is_std_collections_priority_queue_handle_type(type);
}

bool is_std_collections_zone_handle_type(const IrType& type) {
    return is_std_collections_set_handle_type(type) ||
           is_std_collections_iter_handle_type(type) ||
           is_std_collections_map_update_entry_handle_type(type) ||
           is_std_collections_hash_map_handle_type(type) ||
           is_std_collections_hash_set_handle_type(type) ||
           is_std_collections_tree_map_handle_type(type) ||
           is_std_collections_tree_set_handle_type(type) ||
           is_std_collections_deque_handle_type(type) ||
           is_std_collections_ring_buffer_handle_type(type) ||
           is_std_collections_linked_list_handle_type(type) ||
           is_std_collections_binary_heap_handle_type(type) ||
           is_std_collections_priority_queue_handle_type(type);
}

std::string std_collections_handle_display_name(const IrType& type) {
    return collection_type_display_name(value_qualified_set_type(type));
}

std::optional<std::size_t> std_collections_set_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_collections_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) {
        if (is_std_collections_map_update_entry_handle_type(type)) {
            return 0;
        }
        if (is_std_collections_hash_map_handle_type(type) ||
            is_std_collections_hash_set_handle_type(type) ||
            is_std_collections_tree_map_handle_type(type) ||
            is_std_collections_tree_set_handle_type(type) ||
            is_std_collections_binary_heap_handle_type(type) ||
            is_std_collections_priority_queue_handle_type(type)) {
            return 1;
        }
        return 0;
    }

    if (is_std_collections_map_update_entry_handle_type(type)) {
        std::optional<std::size_t> map_index;
        bool has_key = false;
        for (std::size_t i = 0; i < type.field_names.size(); ++i) {
            const std::string& name = type.field_names[i];
            const IrType& field_type = type.field_types[i];
            if (name == "map") {
                if (field_type.qualifier != TypeQualifier::Ptr ||
                    field_type.primitive != IrPrimitiveKind::Struct) {
                    return std::nullopt;
                }
                map_index = i;
            } else if (name == "key") {
                if (!type.args.empty()) {
                    IrType expected = type.args[0];
                    expected.qualifier = TypeQualifier::Value;
                    if (!same_type(field_type, expected)) return std::nullopt;
                }
                has_key = true;
            } else {
                return std::nullopt;
            }
        }
        if (!map_index || !has_key) return std::nullopt;
        return map_index;
    }

    std::optional<std::size_t> data_index;
    std::optional<std::size_t> keys_index;
    bool has_len = false;
    bool has_capacity = false;
    bool has_index = false;
    bool has_hash_or_less = false;
    std::optional<std::size_t> values_index;
    bool has_states = false;
    bool has_left = false;
    bool has_right = false;
    bool has_parent = false;
    bool has_colors = false;
    bool has_root = false;
    bool has_current = false;
    bool has_head = false;
    bool has_tail = false;
    bool has_next_slot = false;
    bool has_free = false;
    bool has_next_node = false;
    bool has_prev = false;
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
            std::size_t value_arg_index =
                (is_std_collections_hash_map_handle_type(type) ||
                 is_std_collections_tree_map_handle_type(type)) ? 1 : 0;
            if (!ptr_field_matches_type_arg(field_type, type, value_arg_index)) return std::nullopt;
            values_index = i;
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
        } else if (name == "next_node") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_next_node = true;
        } else if (name == "prev") {
            if (!is_i64_ptr_type(field_type)) return std::nullopt;
            has_prev = true;
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
        } else if (name == "current") {
            if (!is_std_collections_iter_handle_type(type)) return std::nullopt;
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_current = true;
        } else if (name == "head") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_head = true;
        } else if (name == "tail") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_tail = true;
        } else if (name == "next_slot") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_next_slot = true;
        } else if (name == "free") {
            if (!is_i64_value_type(field_type)) return std::nullopt;
            has_free = true;
        } else {
            return std::nullopt;
        }
    }

    if (is_std_collections_set_handle_type(type)) {
        if (!data_index || !has_len || !has_capacity) return std::nullopt;
        return data_index;
    }
    if (is_std_collections_iter_handle_type(type)) {
        if (is_std_collections_hash_map_keys_handle_type(type)) {
            if (!keys_index || !has_states || !has_capacity || !has_index) return std::nullopt;
            return keys_index;
        }
        if (is_std_collections_hash_map_values_handle_type(type)) {
            if (!values_index || !has_states || !has_capacity || !has_index) return std::nullopt;
            return values_index;
        }
        if (is_std_collections_hash_set_iter_handle_type(type)) {
            if (!data_index || !has_states || !has_capacity || !has_index) return std::nullopt;
            return data_index;
        }
        if (is_std_collections_tree_map_keys_handle_type(type)) {
            if (!keys_index || !has_left || !has_right || !has_parent || !has_current) return std::nullopt;
            return keys_index;
        }
        if (is_std_collections_tree_map_values_handle_type(type)) {
            if (!values_index || !has_left || !has_right || !has_parent || !has_current) return std::nullopt;
            return values_index;
        }
        if (is_std_collections_tree_set_iter_handle_type(type)) {
            if (!data_index || !has_left || !has_right || !has_parent || !has_current) return std::nullopt;
            return data_index;
        }
        if (is_std_collections_deque_iter_handle_type(type) ||
            is_std_collections_ring_buffer_iter_handle_type(type)) {
            if (!data_index || !has_len || !has_capacity || !has_head || !has_index) return std::nullopt;
            return data_index;
        }
        if (is_std_collections_linked_list_iter_handle_type(type)) {
            if (!data_index || !has_next_node || !has_current) return std::nullopt;
            return data_index;
        }
        if (!data_index || !has_len || !has_index) return std::nullopt;
        return data_index;
    }
    if (is_std_collections_hash_map_handle_type(type)) {
        if (!has_hash_or_less || !keys_index || !values_index || !has_states || !has_len || !has_capacity) {
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
        if (!has_hash_or_less || !keys_index || !values_index || !has_left || !has_right ||
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
    if (is_std_collections_deque_handle_type(type) ||
        is_std_collections_ring_buffer_handle_type(type)) {
        if (!data_index || !has_len || !has_capacity || !has_head) return std::nullopt;
        return data_index;
    }
    if (is_std_collections_linked_list_handle_type(type)) {
        if (!data_index || !has_next_node || !has_prev || !has_head || !has_tail ||
            !has_len || !has_capacity || !has_next_slot || !has_free) {
            return std::nullopt;
        }
        return data_index;
    }
    if (is_std_collections_binary_heap_handle_type(type) ||
        is_std_collections_priority_queue_handle_type(type)) {
        if (!has_hash_or_less || !data_index || !has_len || !has_capacity) return std::nullopt;
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
        if (is_std_collections_map_update_entry_handle_type(type)) return {{0}};
        if (is_std_collections_hash_map_handle_type(type)) return {{1}, {2}, {3}};
        if (is_std_collections_hash_map_keys_handle_type(type)) return {{0}, {1}};
        if (is_std_collections_hash_map_values_handle_type(type)) return {{0}, {1}};
        if (is_std_collections_hash_set_handle_type(type)) return {{1}, {2}};
        if (is_std_collections_hash_set_iter_handle_type(type)) return {{0}, {1}};
        if (is_std_collections_tree_map_handle_type(type)) return {{1}, {2}, {3}, {4}, {5}, {6}};
        if (is_std_collections_tree_map_keys_handle_type(type)) return {{0}, {1}, {2}, {3}};
        if (is_std_collections_tree_map_values_handle_type(type)) return {{0}, {1}, {2}, {3}};
        if (is_std_collections_tree_set_handle_type(type)) return {{1}, {2}, {3}, {4}, {5}};
        if (is_std_collections_tree_set_iter_handle_type(type)) return {{0}, {1}, {2}, {3}};
        if (is_std_collections_deque_handle_type(type)) return {{0}};
        if (is_std_collections_deque_iter_handle_type(type)) return {{0}};
        if (is_std_collections_ring_buffer_handle_type(type)) return {{0}};
        if (is_std_collections_ring_buffer_iter_handle_type(type)) return {{0}};
        if (is_std_collections_linked_list_handle_type(type)) return {{0}, {1}, {2}};
        if (is_std_collections_linked_list_iter_handle_type(type)) return {{0}, {1}};
        if (is_std_collections_binary_heap_handle_type(type)) return {{1}};
        if (is_std_collections_priority_queue_handle_type(type)) return {{1}};
        return {{0}};
    }

    if (is_std_collections_map_update_entry_handle_type(type)) {
        std::optional<std::size_t> map_index;
        for (std::size_t i = 0; i < type.field_names.size(); ++i) {
            if (type.field_names[i] == "map") map_index = i;
        }
        std::vector<std::vector<std::size_t>> paths;
        if (map_index) paths.push_back({*map_index});
        return paths;
    }

    std::vector<std::vector<std::size_t>> paths;
    for (std::size_t i = 0; i < type.field_names.size(); ++i) {
        const std::string& name = type.field_names[i];
        if (name == "data" || name == "keys" || name == "values" || name == "states" ||
            name == "left" || name == "right" || name == "parent" || name == "colors" ||
            name == "next_node" || name == "prev") {
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
           method_name == "deque_grow_to_capacity" ||
           method_name == "deque_ensure_spare_capacity" ||
           method_name == "linked_list_grow_to_capacity" ||
           method_name == "linked_list_alloc_node" ||
           method_name == "binary_heap_grow_to_capacity" ||
           method_name == "priority_queue_grow_to_capacity" ||
           method_name == "reserve" ||
           method_name == "reserve_extra" ||
           method_name == "replace" ||
           method_name == "entry" ||
           method_name == "split_off" ||
           method_name == "append" ||
           method_name == "insert" ||
           method_name == "push_back" ||
           method_name == "push_front" ||
           method_name == "push";
}

std::optional<StdCollectionsImplicitZoneMethod> std_collections_implicit_zone_method_for_call(
    const IrType& receiver_type,
    const std::string& method_name,
    std::size_t user_arg_count) {
    IrType receiver_value_type = value_qualified_set_type(receiver_type);
    if (!is_std_collections_mutable_handle_type(receiver_value_type)) return std::nullopt;

    if (is_std_collections_hash_map_handle_type(receiver_value_type) ||
        is_std_collections_tree_map_handle_type(receiver_value_type)) {
        if (method_name == "entry" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"entry", false};
        }
        if (method_name == "insert" && user_arg_count == 2) {
            return StdCollectionsImplicitZoneMethod{"insert", false};
        }
        if (method_name == "split_off" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"split_off", false};
        }
        if (method_name == "append" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"append", false};
        }
        if ((method_name == "reserve" || method_name == "reserve_extra") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        return std::nullopt;
    }

    if (is_std_collections_set_handle_type(receiver_value_type) ||
        is_std_collections_hash_set_handle_type(receiver_value_type) ||
        is_std_collections_tree_set_handle_type(receiver_value_type)) {
        if (method_name == "insert" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"insert", false};
        }
        if (method_name == "replace" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"replace", false};
        }
        if (method_name == "split_off" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"split_off", false};
        }
        if (method_name == "append" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"append", false};
        }
        if (method_name == "reserve" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"reserve", false};
        }
        if (method_name == "reserve_extra" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"reserve_extra", false};
        }
        return std::nullopt;
    }

    if (is_std_collections_deque_handle_type(receiver_value_type)) {
        if ((method_name == "push_back" || method_name == "push_front") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        if ((method_name == "reserve" || method_name == "reserve_extra") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        return std::nullopt;
    }

    if (is_std_collections_linked_list_handle_type(receiver_value_type)) {
        if ((method_name == "push_back" || method_name == "push_front") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        if ((method_name == "reserve" || method_name == "reserve_extra") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        return std::nullopt;
    }

    if (is_std_collections_binary_heap_handle_type(receiver_value_type) ||
        is_std_collections_priority_queue_handle_type(receiver_value_type)) {
        if (method_name == "into_sorted_vec" && user_arg_count == 0) {
            return StdCollectionsImplicitZoneMethod{"into_sorted_vec", false};
        }
        if (method_name == "push" && user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{"push", false};
        }
        if ((method_name == "reserve" || method_name == "reserve_extra") &&
            user_arg_count == 1) {
            return StdCollectionsImplicitZoneMethod{method_name, false};
        }
        return std::nullopt;
    }

    return std::nullopt;
}

bool std_collections_result_preserves_receiver_zone(const IrExpr& call) {
    if (call.kind != IrExprKind::Call || call.args.empty()) return false;
    if (!is_std_collections_mutable_handle_type(value_qualified_set_type(call.args[0]->type))) return false;
    IrType result_type = value_qualified_set_type(call.type);
    return is_std_collections_iter_handle_type(result_type) ||
           is_std_collections_map_update_entry_handle_type(result_type);
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
    if (method_name == "push" &&
        !is_std_collections_binary_heap_handle_type(receiver_value_type) &&
        !is_std_collections_priority_queue_handle_type(receiver_value_type)) {
        return std::nullopt;
    }

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

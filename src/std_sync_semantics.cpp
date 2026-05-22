#include "std_sync_semantics.hpp"

namespace ari {

namespace {

bool is_std_sync_sender_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::sync::Sender";
}

bool is_std_sync_receiver_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::sync::Receiver";
}

bool is_std_sync_channel_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::sync::Channel";
}

bool single_ptr_state_field(const IrType& type) {
    return type.field_names.size() == 1 &&
           type.field_types.size() == 1 &&
           type.field_names[0] == "state" &&
           type.field_types[0].qualifier == TypeQualifier::Ptr;
}

} // namespace

bool is_std_sync_zone_handle_type(const IrType& type) {
    return is_std_sync_sender_type(type) ||
           is_std_sync_receiver_type(type) ||
           is_std_sync_channel_type(type);
}

std::optional<std::size_t> std_sync_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_sync_zone_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (is_std_sync_sender_type(type) || is_std_sync_receiver_type(type)) {
        if (!single_ptr_state_field(type)) return std::nullopt;
        return 0;
    }
    if (is_std_sync_channel_type(type)) {
        if (type.field_names.size() != 2 || type.field_types.size() != 2) return std::nullopt;
        if (type.field_names[0] != "sender" || type.field_names[1] != "receiver") {
            return std::nullopt;
        }
        if (!is_std_sync_sender_type(type.field_types[0]) ||
            !is_std_sync_receiver_type(type.field_types[1])) {
            return std::nullopt;
        }
        return 0;
    }
    return std::nullopt;
}

std::vector<std::vector<std::size_t>> std_sync_zone_handle_storage_field_path_indices(const IrType& type) {
    if (!std_sync_zone_handle_source_field_index(type)) return {};
    if (is_std_sync_channel_type(type)) {
        return {{0}, {1}};
    }
    return {{0}};
}

} // namespace ari

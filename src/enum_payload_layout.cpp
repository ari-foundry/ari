#include "enum_payload_layout.hpp"

#include "type_semantics.hpp"

namespace ari {
namespace {

IrType primitive_type(IrPrimitiveKind primitive, const char* name, SourceLocation loc) {
    IrType type;
    type.qualifier = TypeQualifier::Value;
    type.primitive = primitive;
    type.name = name;
    type.loc = loc;
    return type;
}

bool is_payload_word_storage(const IrType& type) {
    return same_type(type, enum_payload_storage_type(type.loc));
}

} // namespace

IrType enum_tag_storage_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I32, "i32", loc);
}

IrType enum_payload_storage_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::U64, "u64", loc);
}

IrType enum_payload_slot_storage_type(SourceLocation loc, const IrType& payload_type) {
    if (has_aggregate_enum_layout(payload_type)) return payload_type;
    return enum_payload_storage_type(loc);
}

const IrType* enum_payload_slot_scalar_lane_type(const IrType& slot_type) {
    if (!has_aggregate_enum_layout(slot_type) || slot_type.field_types.size() < 2) return nullptr;
    const IrType& lane_type = slot_type.field_types[1];
    return is_payload_word_storage(lane_type) ? &lane_type : nullptr;
}

bool enum_payload_slot_uses_scalar_lane(const IrType& slot_type, const IrType& payload_type) {
    if (same_type(slot_type, enum_payload_slot_storage_type(payload_type.loc, payload_type))) return false;
    if (!enum_payload_slot_scalar_lane_type(slot_type)) return false;
    return same_type(enum_payload_slot_storage_type(payload_type.loc, payload_type),
                     enum_payload_storage_type(payload_type.loc));
}

bool merge_enum_payload_slot_storage_type(SourceLocation loc,
                                          IrType& slot_type,
                                          const IrType& payload_type,
                                          std::string& error) {
    IrType incoming = enum_payload_slot_storage_type(loc, payload_type);
    if (same_type(slot_type, incoming)) return true;

    if (enum_payload_slot_uses_scalar_lane(slot_type, payload_type)) return true;

    if (is_payload_word_storage(slot_type) &&
        has_aggregate_enum_layout(incoming) &&
        enum_payload_slot_scalar_lane_type(incoming)) {
        slot_type = incoming;
        return true;
    }

    error = "enum aggregate payload slot mixes storage types " + type_name(slot_type) +
            " and " + type_name(incoming);
    return false;
}

} // namespace ari

#include "enum_payload_layout.hpp"

#include "layout.hpp"
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

bool is_payload_word_lane_storage(const IrType& type) {
    if (is_payload_word_storage(type)) return true;
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::I64;
}

bool is_inline_payload_storage_type(const IrType& type) {
    if (has_aggregate_enum_layout(type)) return true;
    return type.qualifier == TypeQualifier::Value &&
           (is_float_primitive(type.primitive) ||
            type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            (type.primitive == IrPrimitiveKind::Vector && type.array_size != 0 && !type.field_types.empty()) ||
            type.primitive == IrPrimitiveKind::Struct);
}

bool layout_size_bytes(const IrType& type, std::uint64_t& out) {
    return ari_layout_size_bytes(type, out);
}

bool is_enum_payload_byte_storage_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Array &&
           type.args.size() == 1 &&
           type.args[0].qualifier == TypeQualifier::Value &&
           type.args[0].primitive == IrPrimitiveKind::U8;
}

bool contains_unresolved_generic_type(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::Unknown) return true;
    for (const auto& arg : type.args) {
        if (contains_unresolved_generic_type(arg)) return true;
    }
    for (const auto& field : type.field_types) {
        if (contains_unresolved_generic_type(field)) return true;
    }
    return false;
}

} // namespace

IrType enum_tag_storage_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I32, "i32", loc);
}

IrType enum_payload_storage_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::U64, "u64", loc);
}

IrType enum_payload_byte_storage_type(SourceLocation loc, std::uint64_t size_bytes) {
    IrType type;
    type.qualifier = TypeQualifier::Value;
    type.primitive = IrPrimitiveKind::Array;
    type.name = "$enum_payload_bytes";
    type.args.push_back(primitive_type(IrPrimitiveKind::U8, "u8", loc));
    type.loc = loc;
    type.array_size = size_bytes;
    return type;
}

IrType enum_payload_slot_storage_type(SourceLocation loc, const IrType& payload_type) {
    if (is_owned_word_enum_payload_type(payload_type)) return payload_type;
    // Generic declarations, aliases, and signatures are validated with
    // placeholder IrType::Unknown arguments. Their final aggregate size is
    // only known after monomorphization, so keep the placeholder slot compact
    // and let concrete enum applications recompute exact storage later.
    if (contains_unresolved_generic_type(payload_type)) return enum_payload_storage_type(loc);
    if (is_inline_payload_storage_type(payload_type)) return payload_type;
    return enum_payload_storage_type(loc);
}

const IrType* enum_payload_slot_scalar_lane_type(const IrType& slot_type) {
    std::optional<std::uint32_t> index = enum_payload_slot_scalar_lane_index(slot_type);
    if (!index) return nullptr;
    return &slot_type.field_types[*index];
}

std::optional<std::uint32_t> enum_payload_slot_scalar_lane_index(const IrType& slot_type) {
    if (slot_type.field_types.empty()) return std::nullopt;
    // Mixed payload slots store compact payload words inside a larger
    // aggregate's first word lane, keeping the full aggregate available for
    // the cases that need it.
    if (has_aggregate_enum_layout(slot_type)) {
        if (slot_type.field_types.size() < 2) return std::nullopt;
        return is_payload_word_lane_storage(slot_type.field_types[1])
            ? std::optional<std::uint32_t>{1}
            : std::nullopt;
    }
    if (!is_inline_payload_storage_type(slot_type)) return std::nullopt;
    return is_payload_word_lane_storage(slot_type.field_types[0])
        ? std::optional<std::uint32_t>{0}
        : std::nullopt;
}

bool enum_payload_slot_uses_scalar_lane(const IrType& slot_type, const IrType& payload_type) {
    if (same_type(slot_type, enum_payload_slot_storage_type(payload_type.loc, payload_type))) return false;
    if (!enum_payload_slot_scalar_lane_type(slot_type)) return false;
    return same_type(enum_payload_slot_storage_type(payload_type.loc, payload_type),
                     enum_payload_storage_type(payload_type.loc));
}

bool enum_payload_slot_uses_byte_storage(const IrType& slot_type, const IrType& payload_type) {
    if (same_type(slot_type, enum_payload_slot_storage_type(payload_type.loc, payload_type))) return false;
    if (!is_enum_payload_byte_storage_type(slot_type)) return false;
    std::uint64_t payload_size = 0;
    if (!layout_size_bytes(payload_type, payload_size)) return false;
    return payload_size <= slot_type.array_size;
}

bool merge_enum_payload_slot_storage_type(SourceLocation loc,
                                          IrType& slot_type,
                                          const IrType& payload_type,
                                          std::string& error) {
    IrType incoming = enum_payload_slot_storage_type(loc, payload_type);
    if (same_type(slot_type, incoming)) return true;

    if (enum_payload_slot_uses_scalar_lane(slot_type, payload_type)) return true;
    if (enum_payload_slot_uses_byte_storage(slot_type, payload_type)) return true;

    if (is_payload_word_storage(slot_type) &&
        enum_payload_slot_scalar_lane_type(incoming)) {
        slot_type = incoming;
        return true;
    }

    std::uint64_t slot_size = 0;
    std::uint64_t incoming_size = 0;
    if (layout_size_bytes(slot_type, slot_size) &&
        layout_size_bytes(incoming, incoming_size)) {
        slot_type = enum_payload_byte_storage_type(loc, std::max(slot_size, incoming_size));
        return true;
    }

    error = "enum aggregate payload slot mixes storage types " + type_name(slot_type) +
            " and " + type_name(incoming);
    return false;
}

} // namespace ari

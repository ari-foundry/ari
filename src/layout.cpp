#include "layout.hpp"

#include <limits>

namespace ari {

namespace {

bool is_aggregate_layout_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            type.primitive == IrPrimitiveKind::Struct ||
            (type.primitive == IrPrimitiveKind::Enum && !type.field_types.empty()));
}

bool add_slot_count(std::uint64_t& total, std::uint64_t value) {
    if (value > std::numeric_limits<std::uint64_t>::max() - total) return false;
    total += value;
    return true;
}

bool ari_layout_slot_count(const IrType& type, std::uint64_t& out) {
    if (!is_aggregate_layout_type(type)) {
        out = type.primitive == IrPrimitiveKind::Void && type.qualifier == TypeQualifier::Value ? 0 : 1;
        return true;
    }

    std::uint64_t total = 0;
    if (type.primitive == IrPrimitiveKind::Tuple) {
        for (const auto& item : type.args) {
            std::uint64_t slots = 0;
            if (!ari_layout_slot_count(item, slots) || !add_slot_count(total, slots)) return false;
        }
        out = total;
        return true;
    }

    if (type.primitive == IrPrimitiveKind::Array && type.field_types.empty() && type.args.size() == 1) {
        std::uint64_t element_slots = 0;
        if (!ari_layout_slot_count(type.args[0], element_slots)) return false;
        if (element_slots != 0 &&
            type.array_size > std::numeric_limits<std::uint64_t>::max() / element_slots) {
            return false;
        }
        out = element_slots * type.array_size;
        return true;
    }

    const std::vector<IrType>& fields = type.field_types;
    for (const auto& item : fields) {
        std::uint64_t slots = 0;
        if (!ari_layout_slot_count(item, slots) || !add_slot_count(total, slots)) return false;
    }
    out = total;
    return true;
}

bool scalar_layout_size_bytes(const IrType& type, std::uint64_t& out) {
    if (type.qualifier != TypeQualifier::Value) {
        out = 8;
        return true;
    }

    switch (type.primitive) {
        case IrPrimitiveKind::Void:
            out = 0;
            return true;
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::Bool:
            out = 1;
            return true;
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::U16:
            out = 2;
            return true;
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::F32:
            out = 4;
            return true;
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U64:
        case IrPrimitiveKind::F64:
        case IrPrimitiveKind::String:
        case IrPrimitiveKind::Function:
        case IrPrimitiveKind::Zone:
            out = 8;
            return true;
        case IrPrimitiveKind::F128:
            out = 16;
            return true;
        case IrPrimitiveKind::Enum:
            if (!type.field_types.empty()) return false;
            out = 8;
            return true;
        default:
            return false;
    }
}

bool scalar_layout_align_bytes(const IrType& type, std::uint64_t& out) {
    if (type.qualifier != TypeQualifier::Value) {
        out = 8;
        return true;
    }

    switch (type.primitive) {
        case IrPrimitiveKind::Void:
            out = 1;
            return true;
        case IrPrimitiveKind::I8:
        case IrPrimitiveKind::U8:
        case IrPrimitiveKind::Bool:
            out = 1;
            return true;
        case IrPrimitiveKind::I16:
        case IrPrimitiveKind::U16:
            out = 2;
            return true;
        case IrPrimitiveKind::I32:
        case IrPrimitiveKind::U32:
        case IrPrimitiveKind::F32:
            out = 4;
            return true;
        case IrPrimitiveKind::I64:
        case IrPrimitiveKind::U64:
        case IrPrimitiveKind::F64:
        case IrPrimitiveKind::String:
        case IrPrimitiveKind::Function:
        case IrPrimitiveKind::Zone:
        case IrPrimitiveKind::Enum:
            out = 8;
            return true;
        case IrPrimitiveKind::F128:
            out = 16;
            return true;
        default:
            return false;
    }
}

} // namespace

bool ari_layout_size_bytes(const IrType& type, std::uint64_t& out) {
    if (!is_aggregate_layout_type(type)) return scalar_layout_size_bytes(type, out);

    std::uint64_t slots = 0;
    if (!ari_layout_slot_count(type, slots)) return false;
    if (slots > std::numeric_limits<std::uint64_t>::max() / 8) return false;
    out = slots * 8;
    return true;
}

bool ari_layout_align_bytes(const IrType& type, std::uint64_t& out) {
    if (!is_aggregate_layout_type(type)) return scalar_layout_align_bytes(type, out);

    std::uint64_t slots = 0;
    if (!ari_layout_slot_count(type, slots)) return false;
    out = slots == 0 ? 1 : 8;
    return true;
}

} // namespace ari

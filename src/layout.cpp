#include "layout.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace ari {

namespace {

bool checked_add(std::uint64_t left, std::uint64_t right, std::uint64_t& out) {
    if (right > std::numeric_limits<std::uint64_t>::max() - left) return false;
    out = left + right;
    return true;
}

bool checked_mul(std::uint64_t left, std::uint64_t right, std::uint64_t& out) {
    if (left != 0 && right > std::numeric_limits<std::uint64_t>::max() / left) return false;
    out = left * right;
    return true;
}

bool align_to(std::uint64_t value, std::uint64_t alignment, std::uint64_t& out) {
    if (alignment == 0) return false;
    std::uint64_t remainder = value % alignment;
    if (remainder == 0) {
        out = value;
        return true;
    }
    return checked_add(value, alignment - remainder, out);
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
        case IrPrimitiveKind::StaticStr:
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
        case IrPrimitiveKind::StaticStr:
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

bool aggregate_layout_bytes(const IrType& type,
                            std::uint64_t& size_out,
                            std::uint64_t& align_out,
                            std::vector<std::uint64_t>* offsets) {
    if (!ari_is_aggregate_layout_type(type)) {
        if (offsets) offsets->clear();
        return scalar_layout_size_bytes(type, size_out) &&
               scalar_layout_align_bytes(type, align_out);
    }

    if (type.primitive == IrPrimitiveKind::Array && type.field_types.empty() && type.args.size() == 1) {
        std::uint64_t element_size = 0;
        std::uint64_t element_align = 0;
        if (!aggregate_layout_bytes(type.args[0], element_size, element_align, nullptr)) return false;
        std::uint64_t stride = 0;
        if (!align_to(element_size, element_align, stride)) return false;
        if (!checked_mul(stride, type.array_size, size_out)) return false;
        align_out = element_align;
        if (offsets) {
            offsets->clear();
            offsets->reserve(static_cast<std::size_t>(type.array_size));
            for (std::uint64_t i = 0; i < type.array_size; ++i) {
                std::uint64_t offset = 0;
                if (!checked_mul(stride, i, offset)) return false;
                offsets->push_back(offset);
            }
        }
        return true;
    }

    std::uint64_t offset = 0;
    std::uint64_t max_align = 1;
    if (offsets) offsets->clear();
    for (const auto& field : ari_aggregate_field_types(type)) {
        std::uint64_t field_size = 0;
        std::uint64_t field_align = 0;
        if (!aggregate_layout_bytes(field, field_size, field_align, nullptr)) return false;
        max_align = std::max(max_align, field_align);
        if (!align_to(offset, field_align, offset)) return false;
        if (offsets) offsets->push_back(offset);
        if (!checked_add(offset, field_size, offset)) return false;
    }

    align_out = max_align;
    return align_to(offset, max_align, size_out);
}

} // namespace

const std::vector<IrType>& trait_object_layout_fields(const IrType& type) {
    static const std::vector<IrType> borrowed_fields = [] {
        IrType data;
        data.qualifier = TypeQualifier::Ptr;
        data.primitive = IrPrimitiveKind::Void;
        data.name = "void";

        IrType vtable = data;
        return std::vector<IrType>{data, vtable};
    }();
    static const std::vector<IrType> owned_fields = [] {
        IrType data;
        data.qualifier = TypeQualifier::Ptr;
        data.primitive = IrPrimitiveKind::Void;
        data.name = "void";

        IrType vtable = data;
        IrType drop = data;
        return std::vector<IrType>{data, vtable, drop};
    }();
    return type.qualifier == TypeQualifier::Own ? owned_fields : borrowed_fields;
}

bool ari_has_aggregate_enum_layout(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Enum &&
           !type.field_types.empty();
}

bool ari_is_aggregate_layout_type(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::TraitObject) return true;
    return type.qualifier == TypeQualifier::Value &&
           (type.primitive == IrPrimitiveKind::Tuple ||
            type.primitive == IrPrimitiveKind::Array ||
            (type.primitive == IrPrimitiveKind::Vector && type.field_types.size() == 2) ||
            type.primitive == IrPrimitiveKind::Struct ||
            ari_has_aggregate_enum_layout(type));
}

const std::vector<IrType>& ari_aggregate_field_types(const IrType& type) {
    if (type.primitive == IrPrimitiveKind::TraitObject) return trait_object_layout_fields(type);
    if (type.primitive == IrPrimitiveKind::Struct ||
        type.primitive == IrPrimitiveKind::Array ||
        type.primitive == IrPrimitiveKind::Vector ||
        ari_has_aggregate_enum_layout(type)) return type.field_types;
    return type.args;
}

bool ari_layout_field_count(const IrType& type, std::uint64_t& out) {
    if (!ari_is_aggregate_layout_type(type)) return false;
    if (type.primitive == IrPrimitiveKind::Array && type.field_types.empty() && type.args.size() == 1) {
        out = type.array_size;
        return true;
    }
    out = static_cast<std::uint64_t>(ari_aggregate_field_types(type).size());
    return true;
}

bool ari_layout_size_bytes(const IrType& type, std::uint64_t& out) {
    std::uint64_t align = 0;
    return aggregate_layout_bytes(type, out, align, nullptr);
}

bool ari_layout_align_bytes(const IrType& type, std::uint64_t& out) {
    std::uint64_t size = 0;
    return aggregate_layout_bytes(type, size, out, nullptr);
}

bool ari_layout_field_offset_bytes(const IrType& type, std::uint64_t index, std::uint64_t& out) {
    if (!ari_is_aggregate_layout_type(type)) return false;
    std::vector<std::uint64_t> offsets;
    std::uint64_t size = 0;
    std::uint64_t align = 0;
    if (!aggregate_layout_bytes(type, size, align, &offsets)) return false;
    if (index >= offsets.size()) return false;
    out = offsets[static_cast<std::size_t>(index)];
    return true;
}

} // namespace ari

#include "aggregate_abi.hpp"

#include "layout.hpp"

namespace ari {

bool is_nonlocal_aggregate_abi_type(const IrType& type) {
    if (type.qualifier != TypeQualifier::Value) return false;

    switch (type.primitive) {
        case IrPrimitiveKind::Tuple:
        case IrPrimitiveKind::Array:
        case IrPrimitiveKind::Struct:
        case IrPrimitiveKind::Vector:
            return true;
        case IrPrimitiveKind::Enum:
            return ari_has_aggregate_enum_layout(type);
        default:
            return false;
    }
}

NonLocalAggregateAbi classify_nonlocal_aggregate_abi(const IrType& type, const TargetInfo& target) {
    NonLocalAggregateAbi result;
    if (!is_nonlocal_aggregate_abi_type(type)) {
        result.kind = NonLocalAggregateAbiKind::NotAggregate;
        result.reason = NonLocalAggregateAbiReason::NotAggregate;
        return result;
    }

    if (!(target.pointer_bits == 64 && target.unix)) {
        result.kind = NonLocalAggregateAbiKind::Unsupported;
        result.reason = NonLocalAggregateAbiReason::TargetUnsupported;
        return result;
    }

    if (!ari_layout_size_bytes(type, result.size_bytes) ||
        !ari_layout_align_bytes(type, result.align_bytes)) {
        result.kind = NonLocalAggregateAbiKind::Unsupported;
        result.reason = NonLocalAggregateAbiReason::LayoutUnavailable;
        return result;
    }

    if (result.size_bytes == 0) {
        result.kind = NonLocalAggregateAbiKind::Unsupported;
        result.reason = NonLocalAggregateAbiReason::ZeroSized;
        return result;
    }

    if (result.size_bytes <= 16 && result.align_bytes <= 8) {
        result.kind = NonLocalAggregateAbiKind::Direct;
        result.reason = NonLocalAggregateAbiReason::None;
        return result;
    }

    result.kind = NonLocalAggregateAbiKind::Indirect;
    result.reason = NonLocalAggregateAbiReason::IndirectByLayout;
    return result;
}

} // namespace ari

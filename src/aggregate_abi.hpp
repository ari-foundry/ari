#pragma once

#include "ir.hpp"
#include "target.hpp"

#include <cstdint>

namespace ari {

enum class NonLocalAggregateAbiKind {
    NotAggregate,
    Direct,
    Indirect,
    Unsupported,
};

enum class NonLocalAggregateAbiReason {
    None,
    NotAggregate,
    TargetUnsupported,
    LayoutUnavailable,
    ZeroSized,
    IndirectByLayout,
};

struct NonLocalAggregateAbi {
    NonLocalAggregateAbiKind kind = NonLocalAggregateAbiKind::NotAggregate;
    NonLocalAggregateAbiReason reason = NonLocalAggregateAbiReason::NotAggregate;
    std::uint64_t size_bytes = 0;
    std::uint64_t align_bytes = 0;
};

bool is_nonlocal_aggregate_abi_type(const IrType& type);
NonLocalAggregateAbi classify_nonlocal_aggregate_abi(const IrType& type, const TargetInfo& target);

} // namespace ari

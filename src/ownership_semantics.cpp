#include "ownership_semantics.hpp"

#include "layout.hpp"
#include "type_semantics.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ari {

void collect_owned_field_states(const IrType& type,
                                const std::string& path,
                                std::map<std::string, LocalState>& states) {
    if (type.qualifier == TypeQualifier::Own) {
        if (path.empty() &&
            (type.primitive == IrPrimitiveKind::Tuple ||
             type.primitive == IrPrimitiveKind::Array ||
             type.primitive == IrPrimitiveKind::Struct)) {
            IrType value_type = type;
            value_type.qualifier = TypeQualifier::Value;
            collect_owned_field_states(value_type, path, states);
            return;
        }
        if (!path.empty()) states.emplace(path, LocalState::Alive);
        return;
    }
    if (type.qualifier != TypeQualifier::Value) {
        return;
    }

    if (type.primitive == IrPrimitiveKind::Tuple ||
        type.primitive == IrPrimitiveKind::Array ||
        type.primitive == IrPrimitiveKind::Struct) {
        const std::vector<IrType>& fields = ari_aggregate_field_types(type);
        for (std::size_t i = 0; i < fields.size(); ++i) {
            collect_owned_field_states(
                fields[i],
                local_owned_field_path(path, i),
                states);
        }
        return;
    }

    if (type.primitive == IrPrimitiveKind::Vector && type.args.size() == 1 && type.array_size != 0) {
        for (std::uint64_t i = 0; i < type.array_size; ++i) {
            collect_owned_field_states(
                type.args[0],
                local_owned_field_path(path, static_cast<std::size_t>(i)),
                states);
        }
    }
}

void initialize_owned_field_states(LocalInfo& local) {
    local.owned_field_states.clear();
    collect_owned_field_states(local.type, "", local.owned_field_states);
    local.owned_field_states_complete = !local.owned_field_states.empty();
}

bool initialize_owned_field_states_from_direct_enum_constructor(LocalInfo& local,
                                                               const IrExpr& init) {
    if (init.kind != IrExprKind::EnumConstruct) return false;
    if (!has_aggregate_enum_layout(local.type)) return false;
    if (!same_type(local.type, init.type)) return false;
    if (local.type.field_types.empty()) return false;

    local.owned_field_states.clear();
    local.owned_field_states_complete = true;

    const std::size_t payload_slot_count = local.type.field_types.size() - 1;
    const std::size_t payload_count = std::min(payload_slot_count, init.args.size());
    for (std::size_t payload_index = 0; payload_index < payload_count; ++payload_index) {
        const std::size_t storage_field_index = payload_index + 1;
        collect_owned_field_states(
            local.type.field_types[storage_field_index],
            local_owned_field_path("", storage_field_index),
            local.owned_field_states);
    }
    return true;
}

}

#include "ownership_semantics.hpp"

#include "layout.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ari {

void collect_owned_field_states(const IrType& type,
                                const std::string& path,
                                std::map<std::string, LocalState>& states) {
    if (type.qualifier == TypeQualifier::Own) {
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
}

}

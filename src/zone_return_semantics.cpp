#include "zone_return_semantics.hpp"

#include "std_box_semantics.hpp"
#include "std_cell_semantics.hpp"
#include "std_collections_semantics.hpp"
#include "std_fs_semantics.hpp"
#include "std_rc_semantics.hpp"
#include "std_string_semantics.hpp"
#include "std_thread_semantics.hpp"
#include "std_vec_semantics.hpp"

#include <algorithm>

namespace ari {

bool is_zone_value_type(const IrType& type) {
    return type.primitive == IrPrimitiveKind::Zone &&
           (type.qualifier == TypeQualifier::Value ||
            type.qualifier == TypeQualifier::Own);
}

bool is_zone_borrow_type(const IrType& type) {
    return type.primitive == IrPrimitiveKind::Zone &&
           (type.qualifier == TypeQualifier::Ref ||
            type.qualifier == TypeQualifier::MutRef);
}

bool is_zone_source_type(const IrType& type) {
    return is_zone_value_type(type) || is_zone_borrow_type(type);
}

bool is_zone_metadata_type(const IrType& type) {
    IrType value_type = type;
    if (value_type.qualifier == TypeQualifier::Ref ||
        value_type.qualifier == TypeQualifier::MutRef) {
        value_type.qualifier = TypeQualifier::Value;
    }
    return value_type.qualifier == TypeQualifier::Value &&
           value_type.primitive == IrPrimitiveKind::Struct &&
           value_type.name == "std::zone::ZoneMetadata";
}

std::optional<std::size_t> zone_pointer_return_param_index(const std::vector<IrType>& params,
                                                           const IrType& result) {
    if (!is_zone_pointer_return_type(result)) {
        return std::nullopt;
    }

    std::optional<std::size_t> index;
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (!is_zone_borrow_type(params[i]) && !is_zone_metadata_type(params[i])) continue;
        if (index) return std::nullopt;
        index = i;
    }
    return index;
}

bool is_zone_pointer_return_type(const IrType& type) {
    IrType value_type = type;
    if (value_type.qualifier == TypeQualifier::Ref ||
        value_type.qualifier == TypeQualifier::MutRef) {
        value_type.qualifier = TypeQualifier::Value;
    }

    return type.qualifier == TypeQualifier::Ptr ||
           is_std_box_handle_type(value_type) ||
           is_std_cell_zone_handle_type(value_type) ||
           is_std_collections_zone_handle_type(value_type) ||
           is_std_fs_dir_entry_zone_handle_type(value_type) ||
           is_std_rc_handle_type(value_type) ||
           is_std_string_zone_handle_type(value_type) ||
           is_std_thread_zone_handle_type(value_type) ||
           is_std_vec_zone_handle_type(value_type) ||
           std::any_of(value_type.args.begin(), value_type.args.end(), is_zone_pointer_return_type) ||
           std::any_of(value_type.field_types.begin(), value_type.field_types.end(), is_zone_pointer_return_type);
}

} // namespace ari

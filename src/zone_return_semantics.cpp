#include "zone_return_semantics.hpp"

#include "std_box_semantics.hpp"
#include "std_collections_semantics.hpp"
#include "std_string_semantics.hpp"
#include "std_vec_semantics.hpp"

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

std::optional<std::size_t> zone_pointer_return_param_index(const std::vector<IrType>& params,
                                                           const IrType& result) {
    if (result.qualifier != TypeQualifier::Ptr &&
        !is_std_box_handle_type(result) &&
        !is_std_collections_zone_handle_type(result) &&
        !is_std_string_zone_handle_type(result) &&
        !is_std_vec_zone_handle_type(result)) {
        return std::nullopt;
    }

    std::optional<std::size_t> index;
    for (std::size_t i = 0; i < params.size(); ++i) {
        if (!is_zone_borrow_type(params[i])) continue;
        if (index) return std::nullopt;
        index = i;
    }
    return index;
}

} // namespace ari

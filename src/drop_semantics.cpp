#include "drop_semantics.hpp"

namespace ari {

bool is_drop_trait_method_name(const std::string& trait_name, const std::string& method_name) {
    return (trait_name == "Drop" || trait_name == "std::Drop") && method_name == "drop";
}

bool is_valid_drop_method_signature(std::size_t param_count, const IrType& result_type) {
    return param_count == 1 && result_type.primitive == IrPrimitiveKind::Void;
}

std::string invalid_drop_impl_internal_message(const IrType& receiver_type) {
    return "internal error: invalid Drop impl method for " + type_name(receiver_type);
}

std::string ambiguous_drop_impl_message(const IrType& dropped_type) {
    return "Drop impl for type " + type_name(dropped_type) + " is ambiguous";
}

} // namespace ari

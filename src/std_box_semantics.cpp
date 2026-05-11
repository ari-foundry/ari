#include "std_box_semantics.hpp"

#include "type_semantics.hpp"

namespace ari {

namespace {

IrType value_qualified_box_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

} // namespace

bool is_std_box_handle_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::boxed::Box";
}

std::optional<std::size_t> std_box_zone_handle_source_field_index(const IrType& type) {
    if (!is_std_box_handle_type(type)) return std::nullopt;
    if (type.field_names.empty() && type.field_types.empty()) return 0;
    if (type.field_names.size() != 1 || type.field_types.size() != 1) return std::nullopt;
    if (type.field_names[0] != "data") return std::nullopt;
    const IrType& field_type = type.field_types[0];
    if (field_type.qualifier != TypeQualifier::Ptr) return std::nullopt;
    if (!type.args.empty()) {
        IrType expected_data = type.args[0];
        expected_data.qualifier = TypeQualifier::Ptr;
        if (!same_type(field_type, expected_data)) return std::nullopt;
    }
    return 0;
}

bool std_box_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_std_box_handle_type(value_qualified_box_type(call.args[0]->type));
}

} // namespace ari

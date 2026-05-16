#include "std_box_semantics.hpp"

#include "type_semantics.hpp"

#include <vector>

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

std::optional<std::vector<std::size_t>> std_box_zone_handle_data_field_path_indices(const IrType& type) {
    std::optional<std::size_t> data_index = std_box_zone_handle_source_field_index(type);
    if (!data_index) return std::nullopt;
    return std::vector<std::size_t>{*data_index};
}

bool std_box_method_requires_same_zone_argument(const std::string& method_name) {
    return method_name == "put_in";
}

bool std_box_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_std_box_handle_type(value_qualified_box_type(call.args[0]->type));
}

std::optional<std::string> std_box_same_zone_method_violation(
    const std::string& method_name,
    const IrType& receiver_type,
    const std::vector<IrExprPtr>& args,
    const StdBoxZoneSourceLookup& receiver_zone_source,
    const StdBoxZoneSourceLookup& argument_zone_source) {
    if (!std_box_method_requires_same_zone_argument(method_name) || args.size() < 2) return std::nullopt;
    if (!is_std_box_handle_type(value_qualified_box_type(receiver_type))) return std::nullopt;

    std::string box_source;
    if (!receiver_zone_source(*args[0], box_source)) {
        return "std::boxed::Box." + method_name + " receiver must come from a tracked zone allocation";
    }

    std::string zone_source;
    if (!argument_zone_source(*args[1], zone_source)) {
        return "std::boxed::Box." + method_name + " requires an explicit zone borrow argument";
    }

    if (box_source != zone_source) {
        return "std::boxed::Box." + method_name + " zone argument must match the box allocation zone";
    }

    return std::nullopt;
}

} // namespace ari

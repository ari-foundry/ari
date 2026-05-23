#include "slice_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <utility>

namespace ari {
namespace {

IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
    IrType type;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType i64_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I64, "i64", loc);
}

IrType value_qualified_slice_type(IrType type) {
    type.qualifier = TypeQualifier::Value;
    return type;
}

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(loc, message);
}

} // namespace

bool is_prelude_slice_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           type.name == "std::Slice" &&
           type.args.size() == 1 &&
           type.field_names.size() == 2 &&
           type.field_names[0] == "data" &&
           type.field_names[1] == "len";
}

bool is_owner_element_slice_type(const IrType& type) {
    IrType shape = value_qualified_slice_type(type);
    return is_prelude_slice_type(shape) &&
           !shape.args.empty() &&
           is_owner_type(shape.args[0]);
}

IrType make_prelude_slice_type(SourceLocation loc, const IrType& element) {
    IrType type = primitive_type(IrPrimitiveKind::Struct, "std::Slice", loc);
    IrType data = element;
    data.qualifier = TypeQualifier::Ptr;
    type.args.push_back(element);
    type.field_names.push_back("data");
    type.field_types.push_back(data);
    type.field_mutable.push_back(false);
    type.field_names.push_back("len");
    type.field_types.push_back(i64_type(loc));
    type.field_mutable.push_back(false);
    return type;
}

bool slice_pointer_result_preserves_receiver_zone(const IrExpr& call) {
    return call.kind == IrExprKind::Call &&
           call.type.qualifier == TypeQualifier::Ptr &&
           !call.args.empty() &&
           is_prelude_slice_type(value_qualified_slice_type(call.args[0]->type));
}

void require_slice_element_materializable(SourceLocation loc,
                                          const IrType& element_type,
                                          const std::string& operation) {
    if (element_type.primitive == IrPrimitiveKind::Unknown) return;
    if (!is_raw_pointer_deref_value_type(element_type)) {
        fail(loc, operation + " currently supports scalar or aggregate element types, got " + type_name(element_type));
    }
    if (is_aggregate_type(element_type) &&
        (is_owner_type(element_type) || contains_borrow_type(element_type))) {
        fail(loc, operation + " cannot copy ownership- or borrow-valued aggregate elements yet");
    }
}

} // namespace ari

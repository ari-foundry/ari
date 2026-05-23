#include "value_construction_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

#include <memory>
#include <utility>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(loc, message);
}

} // namespace

const IrType* tuple_literal_expected_element_type(const IrType* expected,
                                                  std::size_t arity,
                                                  std::size_t index) {
    if (!expected || expected->qualifier != TypeQualifier::Value) return nullptr;
    if (expected->primitive == IrPrimitiveKind::Tuple && expected->args.size() == arity) {
        return index < expected->args.size() ? &expected->args[index] : nullptr;
    }
    return nullptr;
}

const IrType* vector_literal_expected_element_type(const IrType* expected) {
    if (!expected || expected->qualifier != TypeQualifier::Value || expected->args.size() != 1) {
        return nullptr;
    }
    if (expected->primitive != IrPrimitiveKind::Array &&
        expected->primitive != IrPrimitiveKind::Vector) {
        return nullptr;
    }
    return &expected->args[0];
}

void require_plain_prelude_aggregate_element(SourceLocation loc,
                                             const IrType& type,
                                             const std::string& aggregate) {
    if (is_void_value_type(type)) fail(loc, aggregate + " literals cannot store void values");
}

bool expected_type_matches_struct_literal(const IrType* expected,
                                          const std::string& struct_name,
                                          std::size_t generic_arity,
                                          std::size_t field_count) {
    return expected &&
           expected->qualifier == TypeQualifier::Value &&
           expected->primitive == IrPrimitiveKind::Struct &&
           expected->name == struct_name &&
           expected->args.size() == generic_arity &&
           expected->field_types.size() == field_count;
}

bool expected_type_matches_enum_constructor(const IrType* expected,
                                            const std::string& enum_name,
                                            std::size_t generic_arity) {
    return expected &&
           expected->qualifier == TypeQualifier::Value &&
           expected->primitive == IrPrimitiveKind::Enum &&
           expected->name == enum_name &&
           expected->args.size() == generic_arity;
}

IrExprPtr make_enum_constructor_ir(SourceLocation loc,
                                   const EnumConstructorIrInfo& info,
                                   std::vector<IrExprPtr> payloads) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->loc = loc;
    lowered->kind = IrExprKind::EnumConstruct;
    lowered->type = info.enum_type;
    set_ir_expr_enum_case(*lowered, info.enum_name, info.case_name);
    IrType payload_type = info.payload_types.empty() ? IrType{} : info.payload_types[0];
    set_ir_expr_enum_result_payload(
        *lowered,
        info.tag,
        !info.payload_types.empty(),
        std::move(payload_type));

    if (has_aggregate_enum_layout(info.enum_type)) {
        lowered->args = std::move(payloads);
        return lowered;
    }

    if (!payloads.empty()) {
        set_ir_expr_payload(*lowered, std::move(payloads[0]));
    }
    return lowered;
}

} // namespace ari

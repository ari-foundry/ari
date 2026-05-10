#include "aggregate_literal_semantics.hpp"

#include "common.hpp"
#include "type_semantics.hpp"

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
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

} // namespace ari

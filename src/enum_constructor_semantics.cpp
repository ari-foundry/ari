#include "enum_constructor_semantics.hpp"

namespace ari {

bool expected_type_matches_enum_constructor(const IrType* expected,
                                            const std::string& enum_name,
                                            std::size_t generic_arity) {
    return expected &&
           expected->qualifier == TypeQualifier::Value &&
           expected->primitive == IrPrimitiveKind::Enum &&
           expected->name == enum_name &&
           expected->args.size() == generic_arity;
}

} // namespace ari

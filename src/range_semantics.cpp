#include "range_semantics.hpp"

#include "type_semantics.hpp"

#include <cstddef>
#include <utility>

namespace ari {
namespace {

std::string unqualified_name(const std::string& name) {
    std::size_t split = name.rfind("::");
    if (split == std::string::npos) return name;
    return name.substr(split + 2);
}

IrType primitive_type(IrPrimitiveKind primitive, std::string name, SourceLocation loc) {
    IrType type;
    type.primitive = primitive;
    type.name = std::move(name);
    type.loc = loc;
    return type;
}

IrType i64_type(SourceLocation loc) {
    return primitive_type(IrPrimitiveKind::I64, primitive_name(IrPrimitiveKind::I64), loc);
}

} // namespace

bool is_prelude_range_type_name(const std::string& name) {
    std::string base = unqualified_name(name);
    return base == "Range" || base == "RangeInclusive";
}

bool is_prelude_range_type(const IrType& type) {
    return type.qualifier == TypeQualifier::Value &&
           type.primitive == IrPrimitiveKind::Struct &&
           (type.name == "Range" || type.name == "RangeInclusive") &&
           type.args.size() == 1;
}

IrType make_prelude_range_type(SourceLocation loc, bool inclusive, IrType bound) {
    IrType type = primitive_type(
        IrPrimitiveKind::Struct,
        inclusive ? "RangeInclusive" : "Range",
        loc
    );
    type.args.push_back(bound);
    type.field_names.push_back("start");
    type.field_types.push_back(bound);
    type.field_mutable.push_back(false);
    type.field_names.push_back("end");
    type.field_types.push_back(bound);
    type.field_mutable.push_back(false);
    return type;
}

IrType make_prelude_range_type(SourceLocation loc, bool inclusive) {
    return make_prelude_range_type(loc, inclusive, i64_type(loc));
}

} // namespace ari

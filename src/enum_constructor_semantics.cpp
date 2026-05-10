#include "enum_constructor_semantics.hpp"

#include "type_semantics.hpp"

#include <memory>
#include <utility>

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

IrExprPtr make_enum_constructor_ir(SourceLocation loc,
                                   const EnumConstructorIrInfo& info,
                                   std::vector<IrExprPtr> payloads) {
    auto lowered = std::make_unique<IrExpr>();
    lowered->loc = loc;
    lowered->kind = IrExprKind::EnumConstruct;
    lowered->type = info.enum_type;
    lowered->enum_name = info.enum_name;
    lowered->case_name = info.case_name;
    lowered->enum_tag = info.tag;
    lowered->has_payload = !info.payload_types.empty();

    if (has_aggregate_enum_layout(info.enum_type)) {
        lowered->args = std::move(payloads);
        return lowered;
    }

    if (!payloads.empty()) {
        lowered->payload_type = info.payload_types[0];
        lowered->payload = std::move(payloads[0]);
    }
    return lowered;
}

} // namespace ari

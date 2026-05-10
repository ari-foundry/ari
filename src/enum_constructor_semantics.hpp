#pragma once

#include "ir.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct EnumConstructorIrInfo {
    std::string enum_name;
    std::string case_name;
    std::uint32_t tag = 0;
    IrType enum_type;
    std::vector<IrType> payload_types;
};

bool expected_type_matches_enum_constructor(const IrType* expected,
                                            const std::string& enum_name,
                                            std::size_t generic_arity);

IrExprPtr make_enum_constructor_ir(SourceLocation loc,
                                   const EnumConstructorIrInfo& info,
                                   std::vector<IrExprPtr> payloads);

} // namespace ari

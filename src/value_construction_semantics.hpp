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

const IrType* tuple_literal_expected_element_type(const IrType* expected,
                                                  std::size_t arity,
                                                  std::size_t index);

const IrType* vector_literal_expected_element_type(const IrType* expected);

void require_plain_prelude_aggregate_element(SourceLocation loc,
                                             const IrType& type,
                                             const std::string& aggregate);

bool expected_type_matches_struct_literal(const IrType* expected,
                                          const std::string& struct_name,
                                          std::size_t generic_arity,
                                          std::size_t field_count);

bool expected_type_matches_enum_constructor(const IrType* expected,
                                            const std::string& enum_name,
                                            std::size_t generic_arity);

IrExprPtr make_enum_constructor_ir(SourceLocation loc,
                                   const EnumConstructorIrInfo& info,
                                   std::vector<IrExprPtr> payloads);

} // namespace ari

#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <map>
#include <string>
#include <vector>

namespace ari {

void bind_generic_type(SourceLocation loc,
                       const std::string& name,
                       const IrType& binding,
                       std::map<std::string, IrType>& substitutions);

bool infer_generic_pattern_type(const IrType& pattern,
                                const IrType& actual,
                                const std::vector<std::string>& generic_names,
                                std::map<std::string, IrType>& substitutions);

IrType substitute_inferred_type(const IrType& type,
                                const std::map<std::string, IrType>& substitutions);

void infer_named_generic_type(SourceLocation loc,
                              const TypeRef& expected,
                              const IrType& actual,
                              const std::vector<std::string>& generic_names,
                              std::map<std::string, IrType>& substitutions);

void infer_generic_type(SourceLocation loc,
                        const TypeRef& expected,
                        const IrType& actual,
                        const std::vector<GenericParam>& generics,
                        std::map<std::string, IrType>& substitutions);

} // namespace ari

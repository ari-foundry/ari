#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <functional>
#include <optional>
#include <string>

namespace ari {

using CExportTypeResolver = std::function<IrType(const TypeRef&)>;
using CExportEnumTypeLookup = std::function<std::optional<IrType>(const std::string&)>;

std::optional<IrCRecord> build_c_export_record(const StructDecl& decl,
                                               const CExportTypeResolver& resolve_type);
std::optional<IrCEnum> build_c_export_enum(const EnumDecl& decl,
                                           const CExportEnumTypeLookup& lookup_enum_type);

} // namespace ari

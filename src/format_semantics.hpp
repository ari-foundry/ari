#pragma once

#include "common.hpp"
#include "ir.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace ari {

enum class FormatInAppendKind {
    String,
    I64,
    U64,
    Bool,
    F32,
    F64,
    Display
};

struct FormatInAppendTarget {
    FormatInAppendKind kind = FormatInAppendKind::I64;
    std::string display_trait_name;
};

struct ParsedFormatString {
    std::vector<std::string> parts;
    std::vector<IrFormatSpec> specs;
};

std::optional<FormatInAppendTarget> builtin_format_in_append_target_from_type(const IrType& type);
FormatInAppendTarget format_in_display_append_target(std::string trait_name);
bool format_in_append_target_is_float(const FormatInAppendTarget& target);
bool format_in_append_target_is_display(const FormatInAppendTarget& target);
const char* format_in_builtin_append_method_name(const FormatInAppendTarget& target);
std::string unsupported_format_in_value_message(const IrType& type);
ParsedFormatString parse_format_string(SourceLocation loc,
                                        const std::string& text,
                                        std::size_t arg_count);

} // namespace ari

#pragma once

#include "common.hpp"
#include "ir.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ari {

struct ParsedFormatString {
    std::vector<std::string> parts;
    std::vector<IrFormatSpec> specs;
};

ParsedFormatString parse_format_string(SourceLocation loc,
                                        const std::string& text,
                                        std::size_t arg_count);

} // namespace ari

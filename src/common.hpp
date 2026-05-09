#pragma once

#include <stdexcept>
#include <string>

namespace ari {

struct CompileError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct SourceLocation {
    int line = 1;
    int column = 1;
};

std::string where(const SourceLocation& loc);

} // namespace ari

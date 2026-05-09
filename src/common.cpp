#include "common.hpp"

namespace ari {

std::string where(const SourceLocation& loc) {
    return std::to_string(loc.line) + ":" + std::to_string(loc.column);
}

} // namespace ari

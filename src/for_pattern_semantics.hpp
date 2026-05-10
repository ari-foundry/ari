#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <cstddef>
#include <functional>
#include <string>

namespace ari {

struct ForPatternStructInfo {
    std::string name;
    bool tuple_struct = false;
};

struct ForPatternValidationHooks {
    std::function<ForPatternStructInfo(SourceLocation, const std::string&, const IrType&)>
        require_struct_pattern_type;
    std::function<std::size_t(SourceLocation, const IrType&, const std::string&)>
        struct_field_index;
};

void require_irrefutable_non_iterator_for_pattern(const Pattern& pattern,
                                                  const IrType& value_type,
                                                  const ForPatternValidationHooks& hooks);

} // namespace ari

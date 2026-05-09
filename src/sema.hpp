#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <set>
#include <string>

namespace ari {

struct SemaOptions {
    bool require_main = true;
    bool test_mode = false;
    std::set<std::string> cfg_features;
};

IrProgram check_program(const Program& program, SemaOptions options = {});

} // namespace ari

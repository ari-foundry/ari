#pragma once

#include "ast.hpp"
#include "ir.hpp"

#include <set>
#include <string>

namespace ari {

struct SemaOptions {
    bool require_main = true;
    bool test_mode = false;
    bool implicit_std = true;
    std::set<std::string> cfg_features;
    std::set<std::string> cached_ir_function_names;
    std::string target_triple;
};

IrProgram check_program(const Program& program, SemaOptions options = {});

} // namespace ari

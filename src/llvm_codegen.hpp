#pragma once

#include "ir.hpp"

#include <string>

namespace ari {

struct LlvmEmitOptions {
    bool shared_library = false;
    std::string target_triple;
};

std::string emit_llvm_ir(const IrProgram& program, LlvmEmitOptions options = {});

} // namespace ari

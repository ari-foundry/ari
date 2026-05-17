#pragma once

#include "ir.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ari {

struct CodeSymbol {
    std::string name;
    std::uint64_t offset = 0;
    std::uint64_t size = 0;
};

enum class CodeRelocationKind {
    PcRel32Call,
};

struct CodeRelocation {
    CodeRelocationKind kind = CodeRelocationKind::PcRel32Call;
    std::uint64_t offset = 0;
    std::string symbol;
    std::int64_t addend = 0;
};

struct EmittedProgram {
    std::vector<std::uint8_t> code;
    std::vector<CodeSymbol> symbols;
    std::vector<CodeRelocation> relocations;
};

EmittedProgram emit_program_with_symbols(const IrProgram& program);
std::vector<std::uint8_t> emit_program(const IrProgram& program);

} // namespace ari

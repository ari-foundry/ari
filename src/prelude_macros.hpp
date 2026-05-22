#pragma once

#include <string>

namespace ari {

enum class PreludeMacroKind {
    None,
    Assert,
    DebugAssert,
    AssertEq,
    AssertNe,
    Panic,
    Todo,
    Unreachable,
    Print,
    Println,
    Eprintln,
    Format,
    FormatIn,
    Matches,
    Box,
    Vec
};

PreludeMacroKind prelude_macro_kind(const std::string& name);
bool is_prelude_macro_name(const std::string& name);
bool is_supported_prelude_macro(PreludeMacroKind kind);
std::string prelude_macro_function_name(PreludeMacroKind kind);

} // namespace ari

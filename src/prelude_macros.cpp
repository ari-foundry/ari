#include "prelude_macros.hpp"

namespace ari {

PreludeMacroKind prelude_macro_kind(const std::string& name) {
    if (name == "assert") return PreludeMacroKind::Assert;
    if (name == "debug_assert") return PreludeMacroKind::DebugAssert;
    if (name == "assert_eq") return PreludeMacroKind::AssertEq;
    if (name == "assert_ne") return PreludeMacroKind::AssertNe;
    if (name == "panic") return PreludeMacroKind::Panic;
    if (name == "todo") return PreludeMacroKind::Todo;
    if (name == "unreachable") return PreludeMacroKind::Unreachable;
    if (name == "print") return PreludeMacroKind::Print;
    if (name == "println") return PreludeMacroKind::Println;
    if (name == "format") return PreludeMacroKind::Format;
    if (name == "format_in") return PreludeMacroKind::FormatIn;
    if (name == "matches") return PreludeMacroKind::Matches;
    if (name == "Box") return PreludeMacroKind::Box;
    if (name == "Vec") return PreludeMacroKind::Vec;
    return PreludeMacroKind::None;
}

bool is_prelude_macro_name(const std::string& name) {
    return prelude_macro_kind(name) != PreludeMacroKind::None;
}

bool is_supported_prelude_macro(PreludeMacroKind kind) {
    switch (kind) {
        case PreludeMacroKind::Assert:
        case PreludeMacroKind::DebugAssert:
        case PreludeMacroKind::AssertEq:
        case PreludeMacroKind::AssertNe:
        case PreludeMacroKind::Panic:
        case PreludeMacroKind::Todo:
        case PreludeMacroKind::Unreachable:
        case PreludeMacroKind::Print:
        case PreludeMacroKind::Println:
        case PreludeMacroKind::Box:
        case PreludeMacroKind::Vec:
            return true;
        default:
            return false;
    }
}

std::string prelude_macro_function_name(PreludeMacroKind kind) {
    switch (kind) {
        case PreludeMacroKind::Assert: return "assert";
        case PreludeMacroKind::DebugAssert: return "debug_assert";
        case PreludeMacroKind::Panic: return "panic";
        case PreludeMacroKind::Todo: return "todo";
        case PreludeMacroKind::Unreachable: return "unreachable";
        case PreludeMacroKind::Print: return "print";
        case PreludeMacroKind::Println: return "println";
        default: return "";
    }
}

} // namespace ari

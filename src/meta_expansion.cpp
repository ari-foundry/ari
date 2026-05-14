#include "meta_expansion.hpp"

#include "common.hpp"
#include "module_path.hpp"
#include "parser.hpp"

#include <string>
#include <utility>
#include <vector>

namespace ari {

namespace {

[[noreturn]] void fail_expansion(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

std::vector<std::string> module_path_for_macro(const ItemMacroInvocation& invocation) {
    if (invocation.module_name.empty()) return {};
    return split_qualified_path(invocation.module_name);
}

Program parse_item_macro_tokens(const ItemMacroInvocation& invocation) {
    std::vector<Token> tokens = invocation.tokens;
    Token end;
    end.kind = TokenKind::End;
    end.loc = invocation.loc;
    tokens.push_back(end);
    return parse_tokens_in_module(std::move(tokens), module_path_for_macro(invocation));
}

void reject_unsupported_item_macro_output(const Program& program, SourceLocation invocation_loc) {
    if (!program.module_imports.empty()) {
        fail_expansion(program.module_imports.front().loc,
                       "item macro identity expansion cannot generate file-backed module imports; use an inline module output or a source-level mod declaration");
    }
    if (!program.item_macros.empty()) {
        fail_expansion(program.item_macros.front().loc,
                       "nested item macro identity expansion is planned but not supported yet");
    }
    if (program.uses.empty() && program.modules.empty() && program.constants.empty() && program.functions.empty() &&
        program.structs.empty() && program.enums.empty() && program.traits.empty() && program.impls.empty()) {
        fail_expansion(invocation_loc,
                       "item macro identity expansion requires at least one generated use, inline module, function, constant, struct, enum, trait, or impl declaration");
    }
}

} // namespace

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation) {
    Program program = parse_item_macro_tokens(invocation);
    reject_unsupported_item_macro_output(program, invocation.loc);

    ItemMacroExpansion expansion;
    expansion.uses = std::move(program.uses);
    for (auto& use : expansion.uses) {
        if (invocation.is_public) use.is_public = true;
    }
    expansion.modules = std::move(program.modules);
    for (auto& decl : expansion.modules) {
        if (invocation.is_public) decl.is_public = true;
    }
    expansion.constants = std::move(program.constants);
    for (auto& constant : expansion.constants) {
        if (invocation.is_public) constant.is_public = true;
    }
    expansion.functions = std::move(program.functions);
    for (auto& fn : expansion.functions) {
        if (fn.meta) {
            fail_expansion(fn.loc, "item macro identity expansion cannot generate meta functions yet");
        }
        if (invocation.is_public) fn.is_public = true;
    }
    expansion.structs = std::move(program.structs);
    for (auto& decl : expansion.structs) {
        if (invocation.is_public) decl.is_public = true;
    }
    expansion.enums = std::move(program.enums);
    for (auto& decl : expansion.enums) {
        if (invocation.is_public) decl.is_public = true;
    }
    expansion.traits = std::move(program.traits);
    for (auto& decl : expansion.traits) {
        if (invocation.is_public) decl.is_public = true;
    }
    expansion.impls = std::move(program.impls);
    for (auto& decl : expansion.impls) {
        if (invocation.is_public) decl.is_public = true;
    }
    return expansion;
}

} // namespace ari

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

SourceLocation impl_location(const ImplDecl& decl) {
    if (decl.has_trait) return decl.trait_type.loc;
    return decl.for_type.loc;
}

void reject_unsupported_item_macro_output(const Program& program, SourceLocation invocation_loc) {
    if (!program.uses.empty()) {
        fail_expansion(program.uses.front().loc,
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated use declarations remain planned");
    }
    if (!program.module_imports.empty()) {
        fail_expansion(program.module_imports.front().loc,
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated module imports remain planned");
    }
    if (!program.modules.empty()) {
        fail_expansion(program.modules.front().loc,
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated modules remain planned");
    }
    if (!program.item_macros.empty()) {
        fail_expansion(program.item_macros.front().loc,
                       "nested item macro identity expansion is planned but not supported yet");
    }
    if (!program.enums.empty()) {
        fail_expansion(program.enums.front().loc,
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated enums remain planned");
    }
    if (!program.traits.empty()) {
        fail_expansion(program.traits.front().loc,
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated traits remain planned");
    }
    if (!program.impls.empty()) {
        fail_expansion(impl_location(program.impls.front()),
                       "item macro identity expansion currently supports function, constant, and struct declarations; generated impls remain planned");
    }
    if (program.constants.empty() && program.functions.empty() && program.structs.empty()) {
        fail_expansion(invocation_loc,
                       "item macro identity expansion requires at least one generated function, constant, or struct declaration");
    }
}

} // namespace

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation) {
    Program program = parse_item_macro_tokens(invocation);
    reject_unsupported_item_macro_output(program, invocation.loc);

    ItemMacroExpansion expansion;
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
    return expansion;
}

} // namespace ari

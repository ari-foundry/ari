#include "meta_expansion.hpp"

#include "ast_clone.hpp"
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

std::vector<std::string> module_path_for_macro(const std::string& module_name) {
    if (module_name.empty()) return {};
    return split_qualified_path(module_name);
}

std::string ast_expression_hygiene_prefix(SourceLocation loc) {
    return "$meta" + std::to_string(loc.line) + "_" + std::to_string(loc.column);
}

Program parse_item_macro_tokens(const std::vector<Token>& source_tokens,
                                const std::string& module_name,
                                SourceLocation loc) {
    std::vector<Token> tokens = source_tokens;
    Token end;
    end.kind = TokenKind::End;
    end.loc = loc;
    tokens.push_back(end);
    return parse_tokens_in_module(std::move(tokens), module_path_for_macro(module_name));
}

std::vector<Token> substitute_input_tokens(const std::vector<Token>& constructor_tokens,
                                           const std::string& input_name,
                                           const std::vector<Token>& input_tokens) {
    std::vector<Token> expanded;
    for (const Token& token : constructor_tokens) {
        if (token.kind == TokenKind::Identifier && token.text == input_name) {
            expanded.insert(expanded.end(), input_tokens.begin(), input_tokens.end());
            continue;
        }
        expanded.push_back(token);
    }
    return expanded;
}

void reject_unsupported_item_macro_output(const Program& program,
                                          SourceLocation invocation_loc,
                                          const std::string& context) {
    if (!program.module_imports.empty()) {
        fail_expansion(program.module_imports.front().loc,
                       context + " cannot generate file-backed module imports; use an inline module output or a source-level mod declaration");
    }
    if (!program.item_macros.empty()) {
        fail_expansion(program.item_macros.front().loc,
                       "nested " + context + " is planned but not supported yet");
    }
    if (program.uses.empty() && program.modules.empty() && program.constants.empty() && program.functions.empty() &&
        program.structs.empty() && program.enums.empty() && program.traits.empty() && program.impls.empty()) {
        fail_expansion(invocation_loc,
                       context + " requires at least one generated use, inline module, function, constant, struct, enum, trait, or impl declaration");
    }
}

ItemMacroExpansion finish_item_macro_expansion(Program program,
                                               const ItemMacroInvocation& invocation,
                                               const std::string& context) {
    reject_unsupported_item_macro_output(program, invocation.loc, context);

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
            fail_expansion(fn.loc, context + " cannot generate meta functions yet");
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

} // namespace

ItemMacroExpansion expand_item_macro_items(const ItemMacroInvocation& invocation) {
    Program program = parse_item_macro_tokens(invocation.tokens, invocation.module_name, invocation.loc);
    return finish_item_macro_expansion(std::move(program), invocation, "item macro identity expansion");
}

ItemMacroExpansion expand_item_macro_token_constructor(const ItemMacroInvocation& invocation,
                                                       const std::string& input_name,
                                                       const Expr& returned_tokens) {
    if (returned_tokens.kind != ExprKind::MacroCall || returned_tokens.name != "tokens" ||
        !returned_tokens.macro_tokens) {
        fail_expansion(returned_tokens.loc, "internal error: expected tokens!(...) token_stream constructor");
    }
    std::vector<Token> output_tokens =
        substitute_input_tokens(*returned_tokens.macro_tokens, input_name, invocation.tokens);
    Program program = parse_item_macro_tokens(output_tokens, invocation.module_name, returned_tokens.loc);
    return finish_item_macro_expansion(std::move(program), invocation, "item macro token_stream output");
}

ItemMacroExpansion expand_item_macro_decl_constructor(const ItemMacroInvocation& invocation,
                                                      const std::string& input_name,
                                                      const Expr& returned_ast) {
    if (returned_ast.kind != ExprKind::MacroCall || returned_ast.name != "decl" || !returned_ast.macro_tokens) {
        fail_expansion(returned_ast.loc, "internal error: expected decl!(...) ast constructor");
    }
    std::vector<Token> output_tokens = substitute_input_tokens(*returned_ast.macro_tokens, input_name, invocation.tokens);
    Program program = parse_item_macro_tokens(output_tokens, invocation.module_name, returned_ast.loc);
    return finish_item_macro_expansion(std::move(program), invocation, "item macro declaration AST output");
}

Pattern expand_pattern_macro_invocation(const Pattern& invocation) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected pattern macro invocation");
    }
    return parse_macro_pattern(invocation.macro_tokens, invocation.loc);
}

Pattern expand_pattern_macro_token_constructor(const Pattern& invocation,
                                               const std::string& input_name,
                                               const Expr& returned_tokens) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected pattern macro invocation");
    }
    if (returned_tokens.kind != ExprKind::MacroCall || returned_tokens.name != "tokens" ||
        !returned_tokens.macro_tokens) {
        fail_expansion(returned_tokens.loc, "internal error: expected tokens!(...) token_stream constructor");
    }
    return parse_macro_pattern(
        substitute_input_tokens(*returned_tokens.macro_tokens, input_name, invocation.macro_tokens),
        returned_tokens.loc);
}

Pattern expand_pattern_macro_constructor(const Pattern& invocation,
                                         const std::string& input_name,
                                         const Expr& returned_ast) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected pattern macro invocation");
    }
    if (returned_ast.kind != ExprKind::MacroCall || returned_ast.name != "pattern" || !returned_ast.macro_tokens) {
        fail_expansion(returned_ast.loc, "internal error: expected pattern!(...) ast constructor");
    }
    return parse_macro_pattern(
        substitute_input_tokens(*returned_ast.macro_tokens, input_name, invocation.macro_tokens),
        returned_ast.loc);
}

TypeRef expand_type_macro_invocation(const TypeRef& invocation) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected type macro invocation");
    }
    return parse_macro_type_ref(invocation.macro_tokens, invocation.loc);
}

TypeRef expand_type_macro_constructor(const TypeRef& invocation,
                                      const std::string& input_name,
                                      const Expr& returned_type) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected type macro invocation");
    }
    if (returned_type.kind != ExprKind::MacroCall || returned_type.name != "type" || !returned_type.macro_tokens) {
        fail_expansion(returned_type.loc, "internal error: expected type!(...) type constructor");
    }
    return parse_macro_type_ref(
        substitute_input_tokens(*returned_type.macro_tokens, input_name, invocation.macro_tokens),
        returned_type.loc);
}

ExprPtr expand_expression_macro_token_constructor(const std::vector<Token>& invocation_tokens,
                                                  SourceLocation invocation_loc,
                                                  const std::string& input_name,
                                                  const Expr& returned_tokens) {
    if (returned_tokens.kind != ExprKind::MacroCall || returned_tokens.name != "tokens" ||
        !returned_tokens.macro_tokens) {
        fail_expansion(returned_tokens.loc, "internal error: expected tokens!(...) token_stream constructor");
    }
    return parse_macro_expression(
        substitute_input_tokens(*returned_tokens.macro_tokens, input_name, invocation_tokens),
        invocation_loc);
}

ExprPtr expand_ast_expression_return(const Expr& returned_ast,
                                      const std::string& input_name,
                                      const Expr& input_ast,
                                      SourceLocation invocation_loc) {
    return clone_expression_tree_substituting_name_hygienic(
        returned_ast,
        input_name,
        input_ast,
        ast_expression_hygiene_prefix(invocation_loc));
}

} // namespace ari

#include "meta_expansion.hpp"

#include "common.hpp"
#include "module_path.hpp"
#include "parser.hpp"

#include <set>
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

std::string parse_derive_path(const Attribute& attr, std::size_t& index) {
    if (index >= attr.args.size() || attr.args[index].kind != TokenKind::Identifier) {
        fail_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
    }
    std::string path = attr.args[index].text;
    ++index;
    while (index < attr.args.size() && attr.args[index].kind == TokenKind::ColonColon) {
        ++index;
        if (index >= attr.args.size() || attr.args[index].kind != TokenKind::Identifier) {
            fail_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
        }
        path += "::" + attr.args[index].text;
        ++index;
    }
    return path;
}

std::string canonical_derive_name(SourceLocation loc, const std::string& path) {
    if (path == "Debug" || path == "std::Debug") return "Debug";
    fail_expansion(loc, "unsupported derive '" + path + "'; supported derives: Debug");
}

std::vector<std::string> derive_names(const std::vector<Attribute>& attributes) {
    std::vector<std::string> names;
    std::set<std::string> seen;
    for (const auto& attr : attributes) {
        if (attr.name != "derive") continue;
        std::size_t index = 0;
        while (index < attr.args.size()) {
            std::string name = canonical_derive_name(attr.args[index].loc, parse_derive_path(attr, index));
            if (!seen.insert(name).second) {
                fail_expansion(attr.loc, "duplicate derive '" + name + "'");
            }
            names.push_back(std::move(name));
            if (index >= attr.args.size()) break;
            if (attr.args[index].kind != TokenKind::Comma) {
                fail_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
            }
            ++index;
            if (index >= attr.args.size()) {
                fail_expansion(attr.loc, "attribute '@derive' expects trait names separated by commas");
            }
        }
    }
    return names;
}

TypeRef simple_type_ref(const std::string& name, SourceLocation loc) {
    TypeRef type;
    type.name = name;
    type.loc = loc;
    return type;
}

std::vector<TypeRef> generic_type_args(const std::vector<GenericParam>& generics) {
    std::vector<TypeRef> args;
    args.reserve(generics.size());
    for (const auto& generic : generics) {
        args.push_back(simple_type_ref(generic.name, generic.loc));
    }
    return args;
}

ImplDecl make_debug_derive_impl(const std::string& type_name,
                                const std::string& module_name,
                                const std::vector<GenericParam>& generics,
                                SourceLocation loc) {
    ImplDecl impl;
    impl.module_name = module_name;
    impl.has_trait = true;
    impl.generics = generics;
    impl.trait_type = simple_type_ref("std::Debug", loc);
    impl.for_type = simple_type_ref(type_name, loc);
    impl.for_type.args = generic_type_args(generics);
    return impl;
}

template <typename Decl>
std::vector<ImplDecl> expand_derive_impls_for_decl(const Decl& decl) {
    std::vector<ImplDecl> impls;
    for (const auto& name : derive_names(decl.attributes)) {
        if (name == "Debug") {
            impls.push_back(make_debug_derive_impl(decl.name, decl.module_name, decl.generics, decl.loc));
        }
    }
    return impls;
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

Pattern expand_pattern_macro_invocation(const Pattern& invocation) {
    if (!invocation.is_macro_invocation) {
        fail_expansion(invocation.loc, "internal error: expected pattern macro invocation");
    }
    return parse_macro_pattern(invocation.macro_tokens, invocation.loc);
}

std::vector<ImplDecl> expand_derive_impls_for_struct(const StructDecl& decl) {
    return expand_derive_impls_for_decl(decl);
}

std::vector<ImplDecl> expand_derive_impls_for_enum(const EnumDecl& decl) {
    return expand_derive_impls_for_decl(decl);
}

} // namespace ari

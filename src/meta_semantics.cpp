#include "meta_semantics.hpp"

#include "type_semantics.hpp"

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

void require_unique_generic_params(const std::vector<GenericParam>& generics,
                                   const std::string& owner_kind,
                                   const std::string& owner_name) {
    std::set<std::string> names;
    for (const auto& generic : generics) {
        if (!names.insert(generic.name).second) {
            fail(generic.loc,
                 "duplicate generic parameter '" + generic.name + "' in " +
                     owner_kind + " '" + owner_name + "'");
        }
    }
}

} // namespace

MetaTransformKind classify_meta_type_ref(const TypeRef& type) {
    if (type.qualifier != TypeQualifier::Value || type.nullable || !type.args.empty()) {
        return MetaTransformKind::None;
    }
    if (type.name == "token_stream") return MetaTransformKind::TokenStream;
    if (type.name == "ast") return MetaTransformKind::Ast;
    if (type.name == "type") return MetaTransformKind::Type;
    return MetaTransformKind::None;
}

std::string meta_type_names() {
    return "token_stream, ast, or type";
}

std::string meta_transform_signature(MetaTransformKind kind) {
    switch (kind) {
        case MetaTransformKind::TokenStream:
            return "token_stream -> token_stream";
        case MetaTransformKind::Ast:
            return "ast -> ast";
        case MetaTransformKind::Type:
            return "type -> type";
        case MetaTransformKind::None:
            break;
    }
    return "unknown";
}

bool meta_transform_can_rewrite_syntax(MetaTransformKind kind) {
    return kind == MetaTransformKind::TokenStream || kind == MetaTransformKind::Ast;
}

bool meta_transform_allowed_at_site(MetaInvocationSite site, MetaTransformKind kind) {
    switch (site) {
        case MetaInvocationSite::Attribute:
        case MetaInvocationSite::ExpressionMacro:
        case MetaInvocationSite::ItemMacro:
            return meta_transform_can_rewrite_syntax(kind);
        case MetaInvocationSite::TypeMacro:
            return kind == MetaTransformKind::Type;
    }
    return false;
}

std::string unknown_meta_invocation_message(MetaInvocationSite site, const std::string& name) {
    switch (site) {
        case MetaInvocationSite::Attribute:
            return "unknown attribute '@" + name +
                   "'; define a meta function with token_stream or ast input to reserve it";
        case MetaInvocationSite::ExpressionMacro:
            return "unknown macro '" + name + "!'";
        case MetaInvocationSite::ItemMacro:
            return "unknown item macro '" + name + "!'";
        case MetaInvocationSite::TypeMacro:
            return "unknown type macro '" + name + "!'";
    }
    return "unknown meta invocation '" + name + "'";
}

std::string meta_invocation_domain_message(MetaInvocationSite site,
                                           const std::string& name,
                                           const std::string& meta_name,
                                           MetaTransformKind kind) {
    std::string signature = meta_transform_signature(kind);
    switch (site) {
        case MetaInvocationSite::Attribute:
            return "attribute '@" + name + "' is reserved by meta function '" + meta_name +
                   "' with " + signature +
                   " domain; attributes require token_stream -> token_stream or ast -> ast";
        case MetaInvocationSite::ExpressionMacro:
            return "macro invocation '" + name + "!' refers to meta function '" + meta_name +
                   "' with " + signature +
                   " domain; expression macros require token_stream -> token_stream or ast -> ast";
        case MetaInvocationSite::ItemMacro:
            return "item macro invocation '" + name + "!' refers to meta function '" + meta_name +
                   "' with " + signature +
                   " domain; item macros require token_stream -> token_stream or ast -> ast";
        case MetaInvocationSite::TypeMacro:
            return "type macro invocation '" + name + "!' refers to meta function '" + meta_name +
                   "' with " + signature + " domain; type macros require type -> type";
    }
    return "meta invocation '" + name + "' has unsupported meta transform domain";
}

std::string meta_invocation_planned_message(MetaInvocationSite site, const std::string& name) {
    switch (site) {
        case MetaInvocationSite::ExpressionMacro:
            return "macro invocation '" + name +
                   "!' requires compile-time token_stream/ast expansion, which is planned but not implemented yet";
        case MetaInvocationSite::ItemMacro:
            return "item macro invocation '" + name +
                   "!' requires compile-time token_stream/ast expansion, which is planned but not implemented yet";
        case MetaInvocationSite::TypeMacro:
            return "type macro invocation '" + name +
                   "!' requires compile-time type expansion, which is planned but not implemented yet";
        case MetaInvocationSite::Attribute:
            break;
    }
    return "meta invocation '" + name + "' requires compile-time expansion, which is planned but not implemented yet";
}

MetaTransformKind validate_meta_function_signature(const FunctionDecl& fn) {
    require_unique_generic_params(fn.generics, "meta function", fn.name);
    if (!fn.generics.empty()) {
        fail(fn.loc,
             "meta functions cannot be generic; define one concrete meta entry point per token_stream, ast, or type transform");
    }
    if (fn.is_variadic) {
        fail(fn.variadic_loc, "meta functions cannot be variadic; define one explicit transform input");
    }
    if (!fn.has_body) fail(fn.loc, "meta functions must have a body");
    if (!fn.has_return_type) fail(fn.loc, "meta functions must declare a meta return type");
    if (fn.params.size() != 1) {
        fail(fn.loc, "meta functions must take exactly one " + meta_type_names() + " parameter");
    }
    const Param& param = fn.params[0];
    if (param.has_pattern) {
        fail(param.pattern.loc, "meta function parameters cannot use patterns");
    }
    MetaTransformKind input_kind = classify_meta_type_ref(param.type);
    if (input_kind == MetaTransformKind::None) {
        fail(param.type.loc,
             "meta function parameters must use " + meta_type_names() + ", got " + type_ref_key(param.type));
    }
    MetaTransformKind output_kind = classify_meta_type_ref(fn.return_type);
    if (output_kind == MetaTransformKind::None) {
        fail(fn.return_type.loc,
             "meta function return type must be " + meta_type_names() + ", got " + type_ref_key(fn.return_type));
    }
    if (input_kind != output_kind) {
        fail(fn.return_type.loc,
             "meta function return type must match its input meta type; use token_stream -> token_stream, ast -> ast, or type -> type");
    }
    if (!fn.body.empty()) {
        fail(fn.body.front()->loc,
             "meta function bodies are reserved for future compile-time evaluation; keep the body empty for now");
    }
    return input_kind;
}

} // namespace ari

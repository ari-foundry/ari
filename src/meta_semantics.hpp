#pragma once

#include "ast.hpp"
#include "common.hpp"

#include <string>

namespace ari {

enum class MetaTransformKind {
    None,
    TokenStream,
    Ast,
    Type,
};

enum class MetaAstReturnKind {
    None,
    Expression,
    ItemDeclarations,
};

enum class MetaInvocationSite {
    Attribute,
    ExpressionMacro,
    ItemMacro,
    PatternMacro,
    TypeMacro,
};

struct MetaFunctionInfo {
    std::string name;
    std::string module_name;
    MetaTransformKind transform_kind = MetaTransformKind::None;
    SourceLocation loc;
    std::string parameter_name;
    MetaAstReturnKind ast_return_kind = MetaAstReturnKind::None;
    const Expr* ast_return = nullptr;
};

MetaTransformKind classify_meta_type_ref(const TypeRef& type);
std::string meta_type_names();
std::string meta_transform_signature(MetaTransformKind kind);
bool meta_transform_can_rewrite_syntax(MetaTransformKind kind);
bool meta_transform_allowed_at_site(MetaInvocationSite site, MetaTransformKind kind);
std::string unknown_meta_invocation_message(MetaInvocationSite site, const std::string& name);
std::string meta_invocation_domain_message(MetaInvocationSite site,
                                           const std::string& name,
                                           const std::string& meta_name,
                                           MetaTransformKind kind);
MetaTransformKind validate_meta_function_signature(const FunctionDecl& fn);
const Expr* meta_function_ast_return(const FunctionDecl& fn);
MetaAstReturnKind meta_function_ast_return_kind(const FunctionDecl& fn);

} // namespace ari

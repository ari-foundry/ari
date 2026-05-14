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

enum class MetaInvocationSite {
    Attribute,
    ExpressionMacro,
    ItemMacro,
    TypeMacro,
};

struct MetaFunctionInfo {
    std::string name;
    std::string module_name;
    MetaTransformKind transform_kind = MetaTransformKind::None;
    SourceLocation loc;
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
std::string meta_invocation_planned_message(MetaInvocationSite site, const std::string& name);
MetaTransformKind validate_meta_function_signature(const FunctionDecl& fn);

} // namespace ari

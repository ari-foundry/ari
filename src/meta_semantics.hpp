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
MetaTransformKind validate_meta_function_signature(const FunctionDecl& fn);

} // namespace ari

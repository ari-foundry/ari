#pragma once

#include "ast.hpp"
#include "common.hpp"

#include <string>

namespace ari {

struct MetaFunctionInfo {
    std::string name;
    std::string module_name;
    SourceLocation loc;
};

enum class MetaTransformKind {
    None,
    TokenStream,
    Ast,
    Type,
};

MetaTransformKind classify_meta_type_ref(const TypeRef& type);
std::string meta_type_names();
void validate_meta_function_signature(const FunctionDecl& fn);

} // namespace ari

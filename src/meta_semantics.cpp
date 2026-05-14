#include "meta_semantics.hpp"

#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace ari {
namespace {

[[noreturn]] void fail(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

std::string type_ref_key(const TypeRef& type) {
    std::string key;
    switch (type.qualifier) {
        case TypeQualifier::Value:
            break;
        case TypeQualifier::Own:
            key += "own ";
            break;
        case TypeQualifier::Ref:
            key += "ref ";
            break;
        case TypeQualifier::MutRef:
            key += "ref mut ";
            break;
        case TypeQualifier::Ptr:
            key += "ptr ";
            break;
    }

    if (type.name == "Array" && type.args.size() == 1) {
        key += "[" + type_ref_key(type.args[0]) + ", " + std::to_string(type.array_size) + "]";
        if (type.nullable) key += "?";
        return key;
    }

    if (type.is_dyn_object) {
        key += "dyn ";
        key += type.name;
        if (!type.args.empty()) {
            key += "[";
            for (std::size_t i = 0; i < type.args.size(); ++i) {
                if (i > 0) key += ", ";
                key += type_ref_key(type.args[i]);
            }
            key += "]";
        }
        if (type.nullable) key += "?";
        return key;
    }

    if (type.name == "fn" && !type.args.empty()) {
        key += "fn(";
        std::size_t param_count = static_cast<std::size_t>(type.array_size);
        if (param_count + 1 > type.args.size()) param_count = type.args.size() - 1;
        for (std::size_t i = 0; i < param_count; ++i) {
            if (i > 0) key += ", ";
            key += type_ref_key(type.args[i]);
        }
        key += ") -> ";
        key += type_ref_key(type.args[param_count]);
        if (type.nullable) key += "?";
        return key;
    }

    if (type.name == "int") key += "i64";
    else if (type.name == "std::Vec" || type.name == "prelude::Vec") key += "Vec";
    else key += type.name;

    if (!type.args.empty()) {
        key += "[";
        for (std::size_t i = 0; i < type.args.size(); ++i) {
            if (i > 0) key += ", ";
            key += type_ref_key(type.args[i]);
        }
        key += "]";
    }
    if (type.nullable) key += "?";
    return key;
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

void validate_meta_function_signature(const FunctionDecl& fn) {
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
}

} // namespace ari

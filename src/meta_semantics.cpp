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

bool is_identity_meta_return(const Stmt& stmt, const Param& param) {
    if (stmt.kind != StmtKind::Return || !stmt.expr) return false;
    return stmt.expr->kind == ExprKind::Name && stmt.expr->name == param.name;
}

const Expr* return_expression(const FunctionDecl& fn) {
    if (fn.body.size() != 1) return nullptr;
    const Stmt& stmt = *fn.body.front();
    if (stmt.kind != StmtKind::Return || !stmt.expr) return nullptr;
    return stmt.expr.get();
}

bool is_decl_ast_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "decl";
}

bool is_pattern_ast_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "pattern";
}

bool is_type_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "type";
}

bool is_token_stream_constructor(const Expr& expr) {
    return expr.kind == ExprKind::MacroCall && expr.name == "tokens";
}

bool supported_token_stream_return_expr(const Expr& expr, std::string& reason) {
    if (!is_token_stream_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "tokens! token_stream constructor requires one or more output tokens";
        return false;
    }
    return true;
}

bool supported_decl_ast_return_expr(const Expr& expr, std::string& reason) {
    if (!is_decl_ast_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "decl! ast constructor requires one or more declaration tokens";
        return false;
    }
    return true;
}

bool supported_pattern_ast_return_expr(const Expr& expr, std::string& reason) {
    if (!is_pattern_ast_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "pattern! ast constructor requires one or more pattern tokens";
        return false;
    }
    return true;
}

bool supported_type_return_expr(const Expr& expr, std::string& reason) {
    if (!is_type_constructor(expr)) return false;
    if (!expr.macro_tokens || expr.macro_tokens->empty()) {
        reason = "type! type constructor requires one or more type tokens";
        return false;
    }
    return true;
}

void collect_pattern_binding_names(const Pattern& pattern, std::set<std::string>& names) {
    switch (pattern.kind) {
        case PatternKind::Binding:
            if (!pattern.payload_name.empty()) names.insert(pattern.payload_name);
            break;
        case PatternKind::Alias:
            if (!pattern.alias_name.empty()) names.insert(pattern.alias_name);
            break;
        case PatternKind::Wildcard:
        case PatternKind::EnumCase:
        case PatternKind::IntegerLiteral:
        case PatternKind::BoolLiteral:
        case PatternKind::Range:
        case PatternKind::Or:
        case PatternKind::Tuple:
        case PatternKind::Array:
        case PatternKind::Struct:
            break;
    }
    if (pattern.has_payload_binding && !pattern.payload_name.empty()) names.insert(pattern.payload_name);
    if (pattern.payload_pattern) collect_pattern_binding_names(*pattern.payload_pattern, names);
    if (pattern.alias_pattern) collect_pattern_binding_names(*pattern.alias_pattern, names);
    for (const auto& alternative : pattern.alternatives) collect_pattern_binding_names(alternative, names);
    for (const auto& element : pattern.elements) collect_pattern_binding_names(element, names);
}

bool supported_ast_return_expr(const Expr& expr,
                               const std::string& input_name,
                               const std::set<std::string>& local_names,
                               std::string& reason) {
    auto require_operand = [&](const ExprPtr& operand) {
        if (!operand) {
            reason = "malformed ast meta return expression";
            return false;
        }
        return supported_ast_return_expr(*operand, input_name, local_names, reason);
    };
    auto require_binary = [&]() {
        if (!expr_left(expr) || !expr_right(expr)) {
            reason = "malformed ast meta return expression";
            return false;
        }
        return supported_ast_return_expr(*expr_left(expr), input_name, local_names, reason) &&
               supported_ast_return_expr(*expr_right(expr), input_name, local_names, reason);
    };
    auto require_index = [&]() {
        if (!expr_operand(expr) || !expr_right(expr)) {
            reason = "malformed ast meta return expression";
            return false;
        }
        return supported_ast_return_expr(*expr_operand(expr), input_name, local_names, reason) &&
               supported_ast_return_expr(*expr_right(expr), input_name, local_names, reason);
    };
    auto require_args = [&]() {
        for (const auto& arg : expr.args) {
            if (!supported_ast_return_expr(*arg, input_name, local_names, reason)) return false;
        }
        return true;
    };
    auto require_expression_only_body = [&](const std::vector<StmtPtr>& body) {
        if (!body.empty()) {
            reason =
                "ast meta control-flow expression returns currently require expression-only arms with no statement bodies";
            return false;
        }
        return true;
    };
    auto require_if_expr = [&]() {
        if (!expr_if_condition(expr) || !expr_if_then_value(expr) || !expr_if_else_value(expr)) {
            reason = "malformed ast meta return expression";
            return false;
        }
        if (!require_expression_only_body(expr_if_then_body(expr)) ||
            !require_expression_only_body(expr_if_else_body(expr))) {
            return false;
        }
        std::set<std::string> then_names = local_names;
        if (expr_if_condition_pattern(expr)) {
            collect_pattern_binding_names(*expr_if_condition_pattern(expr), then_names);
        }
        return supported_ast_return_expr(*expr_if_condition(expr), input_name, local_names, reason) &&
               supported_ast_return_expr(*expr_if_then_value(expr), input_name, then_names, reason) &&
               supported_ast_return_expr(*expr_if_else_value(expr), input_name, local_names, reason);
    };
    auto require_block_expr = [&]() {
        if (!expr_block_value(expr)) {
            reason = "malformed ast meta return expression";
            return false;
        }
        if (!require_expression_only_body(expr_block_body(expr))) return false;
        return supported_ast_return_expr(*expr_block_value(expr), input_name, local_names, reason);
    };
    auto require_match_expr = [&]() {
        if (!expr_match_value(expr)) {
            reason = "malformed ast meta return expression";
            return false;
        }
        if (!supported_ast_return_expr(*expr_match_value(expr), input_name, local_names, reason)) return false;
        for (const auto& arm : expr_match_arms(expr)) {
            if (!arm.value) {
                reason = "malformed ast meta return expression";
                return false;
            }
            std::set<std::string> arm_names = local_names;
            collect_pattern_binding_names(arm.pattern, arm_names);
            if (!supported_ast_return_expr(*arm.value, input_name, arm_names, reason)) return false;
        }
        return true;
    };

    switch (expr.kind) {
        case ExprKind::Integer:
        case ExprKind::Float:
        case ExprKind::String:
        case ExprKind::Bool:
        case ExprKind::Null:
            return true;
        case ExprKind::StructLiteral:
        case ExprKind::Tuple:
        case ExprKind::Vector:
            return require_args();
        case ExprKind::Borrow:
        case ExprKind::Try:
        case ExprKind::Unary:
        case ExprKind::Cast:
            return require_operand(expr_operand(expr));
        case ExprKind::TupleIndex:
        case ExprKind::FieldAccess:
            return require_operand(expr_operand(expr));
        case ExprKind::Index:
            return require_index();
        case ExprKind::Binary:
        case ExprKind::NullCoalesce:
            return require_binary();
        case ExprKind::Call:
            return require_args();
        case ExprKind::MethodCall:
            return require_operand(expr_operand(expr)) && require_args();
        case ExprKind::Name:
            if (expr.name == input_name || local_names.count(expr.name)) {
                return true;
            } else {
                reason =
                    "ast meta expression returns cannot reference names other than the meta input or local control-flow pattern bindings yet";
            }
            return false;
        case ExprKind::Match:
            return require_match_expr();
        case ExprKind::If:
            return require_if_expr();
        case ExprKind::Block:
            return require_block_expr();
        case ExprKind::MacroCall:
            reason =
                "ast meta expression returns cannot call macros; use decl!(...) for item macro declaration output or pattern!(...) for pattern macro output";
            return false;
    }
    reason = "unsupported ast meta return expression";
    return false;
}

MetaAstReturnKind classify_ast_return_expr(const Expr& expr,
                                           const std::string& input_name,
                                           std::string& reason) {
    if (supported_decl_ast_return_expr(expr, reason)) return MetaAstReturnKind::ItemDeclarations;
    if (supported_pattern_ast_return_expr(expr, reason)) return MetaAstReturnKind::Pattern;
    if (is_decl_ast_constructor(expr) || is_pattern_ast_constructor(expr)) return MetaAstReturnKind::None;
    if (supported_ast_return_expr(expr, input_name, {}, reason)) return MetaAstReturnKind::Expression;
    return MetaAstReturnKind::None;
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
        case MetaInvocationSite::PatternMacro:
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
        case MetaInvocationSite::PatternMacro:
            return "unknown pattern macro '" + name + "!'";
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
        case MetaInvocationSite::PatternMacro:
            return "pattern macro invocation '" + name + "!' refers to meta function '" + meta_name +
                   "' with " + signature +
                   " domain; pattern macros require token_stream -> token_stream or ast -> ast";
        case MetaInvocationSite::TypeMacro:
            return "type macro invocation '" + name + "!' refers to meta function '" + meta_name +
                   "' with " + signature + " domain; type macros require type -> type";
    }
    return "meta invocation '" + name + "' has unsupported meta transform domain";
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
    if (fn.body.size() > 1) {
        fail(fn.body[1]->loc,
             "meta function bodies currently allow only an empty body or `return " + param.name +
                 ";` identity body");
    }
    if (!fn.body.empty() && !is_identity_meta_return(*fn.body.front(), param)) {
        const Expr* returned = return_expression(fn);
        if (input_kind == MetaTransformKind::TokenStream && returned) {
            std::string reason;
            if (supported_token_stream_return_expr(*returned, reason)) {
                return input_kind;
            }
            fail(returned->loc, reason.empty()
                                    ? "token_stream meta function bodies currently allow only an empty body, `return " +
                                          param.name + ";` identity body, or tokens!(...) token output"
                                    : reason);
        }
        if (input_kind == MetaTransformKind::Ast && returned) {
            std::string reason;
            if (classify_ast_return_expr(*returned, param.name, reason) != MetaAstReturnKind::None) {
                return input_kind;
            }
            fail(returned->loc, reason);
        }
        if (input_kind == MetaTransformKind::Type && returned) {
            std::string reason;
            if (supported_type_return_expr(*returned, reason)) {
                return input_kind;
            }
            fail(returned->loc, reason.empty()
                                    ? "type meta function bodies currently allow only an empty body, `return " +
                                          param.name + ";` identity body, or type!(...) type output"
                                    : reason);
        }
        if (input_kind == MetaTransformKind::Ast) {
            fail(fn.body.front()->loc,
                 "meta function bodies currently allow only an empty body, `return " + param.name +
                     ";` identity body, an expression return using literals and the meta input for ast -> ast expression macros, decl!(...) for item macro declaration output, or pattern!(...) for pattern macro output");
        }
        if (input_kind == MetaTransformKind::Type) {
            fail(fn.body.front()->loc,
                 "type meta function bodies currently allow only an empty body, `return " + param.name +
                     ";` identity body, or type!(...) type output");
        }
        fail(fn.body.front()->loc,
             "token_stream meta function bodies currently allow only an empty body, `return " + param.name +
                 ";` identity body, or tokens!(...) token output");
    }
    return input_kind;
}

const Expr* meta_function_token_return(const FunctionDecl& fn) {
    if (classify_meta_type_ref(fn.return_type) != MetaTransformKind::TokenStream) return nullptr;
    if (fn.params.size() != 1) return nullptr;
    const Expr* returned = return_expression(fn);
    if (!returned) return nullptr;
    if (returned->kind == ExprKind::Name && returned->name == fn.params.front().name) return nullptr;
    std::string reason;
    if (!supported_token_stream_return_expr(*returned, reason)) return nullptr;
    return returned;
}

const Expr* meta_function_ast_return(const FunctionDecl& fn) {
    if (classify_meta_type_ref(fn.return_type) != MetaTransformKind::Ast) return nullptr;
    if (fn.params.size() != 1) return nullptr;
    const Expr* returned = return_expression(fn);
    if (!returned) return nullptr;
    if (returned->kind == ExprKind::Name && returned->name == fn.params.front().name) return nullptr;
    return returned;
}

MetaAstReturnKind meta_function_ast_return_kind(const FunctionDecl& fn) {
    const Expr* returned = meta_function_ast_return(fn);
    if (!returned || fn.params.empty()) return MetaAstReturnKind::None;
    std::string reason;
    return classify_ast_return_expr(*returned, fn.params.front().name, reason);
}

const Expr* meta_function_type_return(const FunctionDecl& fn) {
    if (classify_meta_type_ref(fn.return_type) != MetaTransformKind::Type) return nullptr;
    if (fn.params.size() != 1) return nullptr;
    const Expr* returned = return_expression(fn);
    if (!returned) return nullptr;
    if (returned->kind == ExprKind::Name && returned->name == fn.params.front().name) return nullptr;
    std::string reason;
    if (!supported_type_return_expr(*returned, reason)) return nullptr;
    return returned;
}

} // namespace ari

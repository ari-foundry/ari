#include "meta_ast_value_eval.hpp"

#include "ast_clone.hpp"
#include "common.hpp"

#include <cstddef>
#include <string>

namespace ari {
namespace {

[[noreturn]] void fail_eval(SourceLocation loc, const std::string& message) {
    throw CompileError(where(loc) + ": " + message);
}

bool is_meta_input_name(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::Name && expr.name == input_name;
}

bool is_free_ast_call(const Expr& expr,
                      const std::string& name,
                      const std::string& input_name,
                      std::size_t arg_count) {
    return expr.kind == ExprKind::Call &&
           expr.name == name &&
           !expr_operand(expr) &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           expr.args.size() == arg_count &&
           !expr.args.empty() &&
           is_meta_input_name(*expr.args.front(), input_name);
}

bool is_ast_method_call(const Expr& expr,
                        const std::string& name,
                        const std::string& input_name,
                        std::size_t arg_count) {
    return expr.kind == ExprKind::MethodCall &&
           expr.name == name &&
           expr_receiver_type_args(expr).empty() &&
           expr_type_args(expr).empty() &&
           expr_operand(expr) &&
           is_meta_input_name(*expr_operand(expr), input_name) &&
           expr.args.size() == arg_count;
}

bool is_ast_kind_expr(const Expr& expr, const std::string& input_name) {
    return is_free_ast_call(expr, "ast_kind", input_name, 1) ||
           is_ast_method_call(expr, "kind", input_name, 0);
}

bool is_ast_is_expr(const Expr& expr, const std::string& input_name) {
    if (is_free_ast_call(expr, "ast_is", input_name, 2)) {
        return expr.args[1] && expr.args[1]->kind == ExprKind::String;
    }
    return is_ast_method_call(expr, "is", input_name, 1) &&
           expr.args[0] &&
           expr.args[0]->kind == ExprKind::String;
}

std::string ast_is_target_kind(const Expr& expr) {
    if (expr.kind == ExprKind::Call) return expr.args[1]->string_value;
    return expr.args[0]->string_value;
}

std::string expression_kind_name(const Expr& expr) {
    switch (expr.kind) {
        case ExprKind::Integer:
            return "integer";
        case ExprKind::Float:
            return "float";
        case ExprKind::String:
            return "string";
        case ExprKind::Bool:
            return "bool";
        case ExprKind::Null:
            return "null";
        case ExprKind::Name:
            return "name";
        case ExprKind::Borrow:
            return "borrow";
        case ExprKind::Unary:
            return "unary";
        case ExprKind::Cast:
            return "cast";
        case ExprKind::Try:
            return "try";
        case ExprKind::NullCoalesce:
            return "null-coalesce";
        case ExprKind::Tuple:
            return "tuple";
        case ExprKind::TupleIndex:
            return "tuple-index";
        case ExprKind::Index:
            return "index";
        case ExprKind::FieldAccess:
            return "field";
        case ExprKind::StructLiteral:
            return expr_receiver_type_args(expr).empty() ? "struct" : "qualified-struct";
        case ExprKind::Vector:
            return "vector";
        case ExprKind::MacroCall:
            return "macro-call";
        case ExprKind::MethodCall:
            return expr_receiver_type_args(expr).empty() ? "method-call" : "qualified-method-call";
        case ExprKind::Match:
            return "match-expr";
        case ExprKind::If:
            return "if-expr";
        case ExprKind::Block:
            return "block-expr";
        case ExprKind::Binary:
            return "binary";
        case ExprKind::Call:
            if (expr_operand(expr)) return "indirect-call";
            return expr_receiver_type_args(expr).empty() ? "call" : "qualified-call";
    }
    return "unknown";
}

bool is_meta_ast_string_expr(const Expr& expr, const std::string& input_name) {
    return expr.kind == ExprKind::String || is_ast_kind_expr(expr, input_name);
}

std::string evaluate_meta_ast_string_expr(const Expr& expr,
                                          const std::string& input_name,
                                          const Expr& input_ast) {
    if (expr.kind == ExprKind::String) return expr.string_value;
    if (is_ast_kind_expr(expr, input_name)) return expression_kind_name(input_ast);
    fail_eval(expr.loc, "internal error: unsupported ast meta string expression");
}

std::string condition_support_message() {
    return "ast expression meta branch conditions currently support bool literals, !, &&, ||, "
           "input.kind() / ast_kind(input), input.is(\"kind\") / ast_is(input, \"kind\"), "
           "and == or != comparisons against string literal expression-kind names";
}

bool supported_comparison(const Expr& left,
                          const Expr& right,
                          TokenKind op,
                          const std::string& input_name,
                          std::string& reason) {
    if (op == TokenKind::EqEq || op == TokenKind::BangEq) {
        if (is_meta_ast_string_expr(left, input_name) && is_meta_ast_string_expr(right, input_name)) {
            return true;
        }
        if (is_supported_meta_ast_expression_condition(left, input_name, reason) &&
            is_supported_meta_ast_expression_condition(right, input_name, reason)) {
            return true;
        }
    }
    reason = condition_support_message();
    return false;
}

bool evaluate_comparison(const Expr& left,
                         const Expr& right,
                         TokenKind op,
                         const std::string& input_name,
                         const Expr& input_ast);

bool evaluate_condition(const Expr& expr,
                        const std::string& input_name,
                        const Expr& input_ast) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return expr.bool_value;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return !evaluate_condition(*expr_operand(expr), input_name, input_ast);
            }
            break;
        case ExprKind::Binary:
            if (!expr_left(expr) || !expr_right(expr)) break;
            if (expr.op == TokenKind::AmpAmp) {
                return evaluate_condition(*expr_left(expr), input_name, input_ast) &&
                       evaluate_condition(*expr_right(expr), input_name, input_ast);
            }
            if (expr.op == TokenKind::PipePipe) {
                return evaluate_condition(*expr_left(expr), input_name, input_ast) ||
                       evaluate_condition(*expr_right(expr), input_name, input_ast);
            }
            return evaluate_comparison(*expr_left(expr), *expr_right(expr), expr.op, input_name, input_ast);
        case ExprKind::Call:
        case ExprKind::MethodCall:
            if (is_ast_is_expr(expr, input_name)) {
                return expression_kind_name(input_ast) == ast_is_target_kind(expr);
            }
            break;
        default:
            break;
    }
    fail_eval(expr.loc, "internal error: unsupported ast meta condition");
}

bool evaluate_comparison(const Expr& left,
                         const Expr& right,
                         TokenKind op,
                         const std::string& input_name,
                         const Expr& input_ast) {
    if (is_meta_ast_string_expr(left, input_name) && is_meta_ast_string_expr(right, input_name)) {
        bool equal =
            evaluate_meta_ast_string_expr(left, input_name, input_ast) ==
            evaluate_meta_ast_string_expr(right, input_name, input_ast);
        return op == TokenKind::EqEq ? equal : !equal;
    }
    bool equal =
        evaluate_condition(left, input_name, input_ast) ==
        evaluate_condition(right, input_name, input_ast);
    return op == TokenKind::EqEq ? equal : !equal;
}

} // namespace

bool is_supported_meta_ast_expression_condition(const Expr& expr,
                                                const std::string& input_name,
                                                std::string& reason) {
    switch (expr.kind) {
        case ExprKind::Bool:
            return true;
        case ExprKind::Unary:
            if (expr.op == TokenKind::Bang && expr_operand(expr)) {
                return is_supported_meta_ast_expression_condition(*expr_operand(expr), input_name, reason);
            }
            break;
        case ExprKind::Binary:
            if (!expr_left(expr) || !expr_right(expr)) {
                reason = "malformed ast expression meta branch condition";
                return false;
            }
            if (expr.op == TokenKind::AmpAmp || expr.op == TokenKind::PipePipe) {
                return is_supported_meta_ast_expression_condition(*expr_left(expr), input_name, reason) &&
                       is_supported_meta_ast_expression_condition(*expr_right(expr), input_name, reason);
            }
            return supported_comparison(*expr_left(expr), *expr_right(expr), expr.op, input_name, reason);
        case ExprKind::Call:
        case ExprKind::MethodCall:
            if (is_ast_is_expr(expr, input_name)) return true;
            break;
        default:
            break;
    }
    reason = condition_support_message();
    return false;
}

bool is_meta_ast_expression_value_helper(const Expr& expr, const std::string& input_name) {
    if (is_ast_kind_expr(expr, input_name) || is_ast_is_expr(expr, input_name)) return true;
    if (expr.kind == ExprKind::Call &&
        (expr.name == "ast_kind" || expr.name == "ast_is") &&
        !expr.args.empty() &&
        is_meta_input_name(*expr.args.front(), input_name)) {
        return true;
    }
    if (expr.kind == ExprKind::MethodCall &&
        (expr.name == "kind" || expr.name == "is") &&
        expr_operand(expr) &&
        is_meta_input_name(*expr_operand(expr), input_name)) {
        return true;
    }
    return false;
}

std::string meta_ast_expression_value_helper_scope_message() {
    return "ast expression value helpers input.kind() / ast_kind(input) and "
           "input.is(\"kind\") / ast_is(input, \"kind\") can only be used in ast meta if conditions";
}

ExprPtr expand_meta_ast_expression_return(const Expr& returned_ast,
                                          const std::string& input_name,
                                          const Expr& input_ast,
                                          const std::string& hygiene_prefix) {
    if (returned_ast.kind == ExprKind::If &&
        expr_if_condition(returned_ast) &&
        !expr_if_condition_pattern(returned_ast)) {
        std::string reason;
        if (is_supported_meta_ast_expression_condition(*expr_if_condition(returned_ast), input_name, reason)) {
            bool take_then = evaluate_condition(*expr_if_condition(returned_ast), input_name, input_ast);
            const std::vector<StmtPtr>& selected_body =
                take_then ? expr_if_then_body(returned_ast) : expr_if_else_body(returned_ast);
            const ExprPtr& selected_value =
                take_then ? expr_if_then_value(returned_ast) : expr_if_else_value(returned_ast);
            if (!selected_value) {
                fail_eval(returned_ast.loc, "malformed ast expression meta if branch");
            }
            if (!selected_body.empty()) {
                fail_eval(returned_ast.loc,
                          "compile-time ast meta if branches cannot contain statement bodies yet; use expression values in each branch");
            }
            return expand_meta_ast_expression_return(*selected_value, input_name, input_ast, hygiene_prefix);
        }
    }
    return clone_expression_tree_substituting_name_hygienic(
        returned_ast,
        input_name,
        input_ast,
        hygiene_prefix);
}

} // namespace ari
